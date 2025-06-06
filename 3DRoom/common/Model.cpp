#include "Model.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// STBI 用於載入紋理圖片
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Model::~Model() {
    Cleanup();
}

bool Model::LoadModel(const std::string& filepath) {
    // 清理之前的資源
    Cleanup();
    
    // 取得檔案目錄
    directory = GetDirectory(filepath);
    
    // TinyObjLoader 變數
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> objMaterials;
    std::string warn, err;
    
    // 載入 OBJ 檔案
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &objMaterials, &warn, &err,
                               filepath.c_str(), directory.c_str());
    
    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }
    
    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
        return false;
    }
    
    if (!ret) {
        std::cerr << "Failed to load model: " << filepath << std::endl;
        return false;
    }
    
    // 檢查是否有頂點資料
    if (attrib.vertices.empty()) {
        std::cerr << "No vertices found in OBJ file!" << std::endl;
        return false;
    }
    
    // 處理材質
    ProcessMaterials(objMaterials);
    
    // 處理每個形狀（網格）
    for (const auto& shape : shapes) {
        ProcessMesh(attrib, shape, objMaterials);
    }
    
    std::cout << "Successfully loaded model: " << filepath << std::endl;
    std::cout << "Meshes: " << meshes.size() << ", Materials: " << materials.size() << std::endl;
    
    return true;
}

void Model::ProcessMaterials(const std::vector<tinyobj::material_t>& objMaterials) {
    materials.reserve(objMaterials.size());
    
    for (const auto& objMat : objMaterials) {
        Material mat;
        mat.name = objMat.name;
        
        // 複製顏色屬性
        for (int i = 0; i < 3; i++) {
            mat.ambient[i] = objMat.ambient[i];
            mat.diffuse[i] = objMat.diffuse[i];
            mat.specular[i] = objMat.specular[i];
        }
        mat.shininess = objMat.shininess;
        
        // 處理透明度
        mat.alpha = objMat.dissolve;  // TinyObjLoader 中的 dissolve 就是透明度
        if (mat.alpha <= 0.0f) mat.alpha = 1.0f;  // 預設為不透明
        
        std::cout << "Material: " << mat.name << ", Alpha: " << mat.alpha << std::endl;
        
        
        // 載入紋理
        if (!objMat.diffuse_texname.empty()) {
            mat.diffuseTexPath = directory + "/" + objMat.diffuse_texname;
            mat.diffuseTexture = LoadTexture(mat.diffuseTexPath);
        }
        
        if (!objMat.normal_texname.empty()) {
            mat.normalTexPath = directory + "/" + objMat.normal_texname;
            mat.normalTexture = LoadTexture(mat.normalTexPath);
        }
        
        if (!objMat.specular_texname.empty()) {
            mat.specularTexPath = directory + "/" + objMat.specular_texname;
            mat.specularTexture = LoadTexture(mat.specularTexPath);
        }
        
        materials.push_back(mat);
    }
}

void Model::ProcessMesh(const tinyobj::attrib_t& attrib,
                       const tinyobj::shape_t& shape,
                       const std::vector<tinyobj::material_t>& objMaterials) {
    Mesh mesh;
    std::unordered_map<std::string, unsigned int> uniqueVertices;

    std::cout << "  Processing mesh with " << shape.mesh.indices.size() / 3 << " faces" << std::endl; // 顯示面數

    // 處理每個面
    for (size_t f = 0; f < shape.mesh.indices.size(); f += 3) { // 一次處理三個索引（一個三角形）
        for (int i = 0; i < 3; ++i) { // 每個頂點
            const auto& index = shape.mesh.indices[f + i];

            Vertex vertex;

            // 位置
            if (index.vertex_index >= 0) {
                vertex.position[0] = attrib.vertices[3 * index.vertex_index + 0];
                vertex.position[1] = attrib.vertices[3 * index.vertex_index + 1];
                vertex.position[2] = attrib.vertices[3 * index.vertex_index + 2];
            } else {
                std::cerr << "    Warning: Vertex position index is negative!" << std::endl;
                vertex.position[0] = vertex.position[1] = vertex.position[2] = 0.0f;
            }

            // 法向量
            if (index.normal_index >= 0) {
                vertex.normal[0] = attrib.normals[3 * index.normal_index + 0];
                vertex.normal[1] = attrib.normals[3 * index.normal_index + 1];
                vertex.normal[2] = attrib.normals[3 * index.normal_index + 2];
            } else {
                // 如果沒有法向量，設為預設值
                std::cerr << "    Warning: Normal index is negative or missing!" << std::endl;
                vertex.normal[0] = 0.0f;
                vertex.normal[1] = 1.0f;
                vertex.normal[2] = 0.0f;
            }

            // 紋理坐標
            if (index.texcoord_index >= 0) {
                vertex.texCoords[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.texCoords[1] = attrib.texcoords[2 * index.texcoord_index + 1];
            } else {
                std::cerr << "    Warning: TexCoord index is negative or missing!" << std::endl;
                vertex.texCoords[0] = 0.0f;
                vertex.texCoords[1] = 0.0f;
            }

            // 建立唯一頂點標識符
            std::ostringstream oss;
            oss << index.vertex_index << "_" << index.normal_index << "_" << index.texcoord_index;
            std::string vertexKey = oss.str();

            // 檢查是否為重複頂點
            if (uniqueVertices.find(vertexKey) == uniqueVertices.end()) {
                uniqueVertices[vertexKey] = static_cast<unsigned int>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);

            }

            mesh.indices.push_back(uniqueVertices[vertexKey]);
        }
    }

    // 設定材質索引
    if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0) {
        mesh.materialIndex = shape.mesh.material_ids[0];
        std::cout << "  Mesh material index: " << mesh.materialIndex << std::endl;
    } else {
        mesh.materialIndex = -1; // 沒有材質
        std::cout << "  Mesh has no material index" << std::endl;
    }

    // 設置 OpenGL 緩衝區
    SetupMesh(mesh);

    meshes.push_back(mesh);
}

void Model::SetupMesh(Mesh& mesh) {
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    
    glBindVertexArray(mesh.VAO);
    
    // 頂點緩衝區
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex),
                 mesh.vertices.data(), GL_STATIC_DRAW);
    
    // 索引緩衝區
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);
    
    // 頂點屬性
    // 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 法向量
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    
    // 紋理坐標
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(3);
    
    glBindVertexArray(0);
}

GLuint Model::LoadTexture(const std::string& path) {
    // 檢查檔案是否存在
    std::ifstream file(path);
    if (!file) {
        std::cout << "Texture file not found: " << path << std::endl;
        return 0;
    }
    file.close();
    
    // 清除之前的 OpenGL 錯誤
    while (glGetError() != GL_NO_ERROR);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    // 檢查 glGenTextures 是否成功
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "Error generating texture: " << error << std::endl;
        return 0;
    }
    
    stbi_set_flip_vertically_on_load(true);
    
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    
    if (data && width > 0 && height > 0) {
        GLenum format;
        GLenum internalFormat;
        
        if (nrComponents == 1) {
            format = GL_RED;
            internalFormat = GL_RED;
        }
        else if (nrComponents == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB8;
        }
        else if (nrComponents == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        } else {
            std::cout << "Unsupported texture format: " << nrComponents << " components in " << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }
        
        // 綁定紋理前確保沒有其他紋理綁定
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // 檢查綁定是否成功
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "Error binding texture: " << error << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }
        
        // 設置紋理參數 (在上傳數據前設置)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // 上傳紋理數據
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        
        // 檢查紋理上傳是否成功
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "Failed to upload texture data: " << error << " for " << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }
        
        // 生成 mipmap
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // 檢查 mipmap 生成是否成功
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "Error generating mipmap: " << error << std::endl;
            // 不返回錯誤，因為紋理本身已經上傳成功
        }
        
        // 解綁紋理
        glBindTexture(GL_TEXTURE_2D, 0);
        
        stbi_image_free(data);
        std::cout << "Successfully loaded texture: " << path << " (ID: " << textureID << ", " << width << "x" << height << ", " << nrComponents << " components)" << std::endl;
        
        return textureID;
        
    } else {
        std::cout << "Failed to load texture data: " << path;
        if (data == nullptr) {
            std::cout << " - STBI error: " << stbi_failure_reason();
        }
        std::cout << std::endl;
        
        if (data) stbi_image_free(data);
        glDeleteTextures(1, &textureID);
        return 0;
    }
}

void Model::Render(GLuint shaderProgram) {
    // 確保 shader 程式是當前使用的
    glUseProgram(shaderProgram);
    // 如果有透明物體，需要啟用混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (size_t i = 0; i < meshes.size(); i++) {
        const Mesh& mesh = meshes[i];

        std::cout << "  Rendering mesh " << i << " (Material Index: " << mesh.materialIndex << ")" << std::endl;

        // 重置紋理單元 (確保乾淨的狀態)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);  // 透明度貼圖
        glBindTexture(GL_TEXTURE_2D, 0);


        // 設置預設值 (如果沒有材質)
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasDiffuseTexture"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasNormalTexture"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasSpecularTexture"), 0);
        glUniform1f(glGetUniformLocation(shaderProgram, "uMaterial.alpha"), 1.0f);

        // 綁定材質
        if (mesh.materialIndex >= 0 && mesh.materialIndex < materials.size()) {
            const Material& material = materials[mesh.materialIndex];

            std::cout << "  Applying material: " << material.name << std::endl;

            // 設定材質屬性 uniform 變數
            GLint ambientLoc = glGetUniformLocation(shaderProgram, "uMaterial.ambient");
            GLint diffuseLoc = glGetUniformLocation(shaderProgram, "uMaterial.diffuse");
            GLint specularLoc = glGetUniformLocation(shaderProgram, "uMaterial.specular");
            GLint shininessLoc = glGetUniformLocation(shaderProgram, "uMaterial.shininess");
            GLint alphaLoc = glGetUniformLocation(shaderProgram, "uMaterial.alpha");

            if (ambientLoc != -1) {
                glUniform4f(ambientLoc, material.ambient[0], material.ambient[1], material.ambient[2], 1.0f);
            }
            if (diffuseLoc != -1) {
                glUniform4f(diffuseLoc, material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f);
            }

            if (specularLoc != -1) {
                glUniform4f(specularLoc, material.specular[0], material.specular[1], material.specular[2], 1.0f);
            }

            if (shininessLoc != -1) {
                glUniform1f(shininessLoc, material.shininess);
            }
            if (alphaLoc != -1) {
                glUniform1f(alphaLoc, material.alpha);  // 設定材質透明度
            }


            // 綁定漫反射紋理
            if (material.diffuseTexture != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.diffuseTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.diffuseTexture"), 0);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasDiffuseTexture"), 1); // 重要！
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasDiffuseTexture"), 0);
                std::cout << "    No diffuse texture" << std::endl;
            }

            // 綁定法線貼圖
            if (material.normalTexture != 0) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, material.normalTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.normalTexture"), 1);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasNormalTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasNormalTexture"), 0);
                std::cout << "    No normal texture" << std::endl;
            }

            // 綁定鏡面反射貼圖
            if (material.specularTexture != 0) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, material.specularTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.specularTexture"), 2);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasSpecularTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasSpecularTexture"), 0);
                std::cout << "    No specular texture" << std::endl;
            }
            // 綁定透明度貼圖
            if (material.alphaTexture != 0) {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, material.alphaTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.alphaTexture"), 3);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasAlphaTexture"), 1);
                std::cout << "    Using alpha texture" << std::endl;
            }

        } else {
            std::cout << "  Mesh has no material assigned" << std::endl;
        }

        // 渲染網格
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 檢查 OpenGL 錯誤
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "  OpenGL error during rendering: " << error << std::endl;
        }
    }

    // 清理紋理綁定
    for (int i = 0; i < 4; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Model::Cleanup() {
    for (auto& mesh : meshes) {
        if (mesh.VAO != 0) glDeleteVertexArrays(1, &mesh.VAO);
        if (mesh.VBO != 0) glDeleteBuffers(1, &mesh.VBO);
        if (mesh.EBO != 0) glDeleteBuffers(1, &mesh.EBO);
    }
    
    for (auto& material : materials) {
        if (material.diffuseTexture != 0) glDeleteTextures(1, &material.diffuseTexture);
        if (material.normalTexture != 0) glDeleteTextures(1, &material.normalTexture);
        if (material.specularTexture != 0) glDeleteTextures(1, &material.specularTexture);
        if (material.alphaTexture != 0) glDeleteTextures(1, &material.alphaTexture);
    }
    
    meshes.clear();
    materials.clear();
}

const Material& Model::GetMaterial(size_t index) const {
    if (index >= materials.size()) {
        throw std::out_of_range("Material index out of range");
    }
    return materials[index];
}

std::string Model::GetDirectory(const std::string& filepath) {
    size_t pos = filepath.find_last_of('/');
    if (pos == std::string::npos) {
        pos = filepath.find_last_of('\\');
    }
    
    if (pos != std::string::npos) {
        return filepath.substr(0, pos);
    }
    
    return ".";
}

void Model::setAutoRotate()
{
    if( !_bautoRotate ) _bautoRotate = true;
}

void Model::update(float dt)
{
    bool shouldMove = _bautoRotate;
    if (_followLight != nullptr) {
        shouldMove = _followLight->isLightOn() && _followLight->isMotionOn();
    }
    
    if (shouldMove && _followLight != nullptr) {
        // 完全同步燈光的運動參數
        float lightClock = _followLight->getClock();
        glm::vec3 lightStartPos = _followLight->getStartPos();
        
        // 使用與燈光完全相同的計算方式
        float currentAngle = lightClock * M_PI_2; // 與燈光相同的角度計算
        float lightRadius = glm::length(lightStartPos); // 燈光的半徑
        
        // 計算同步位置 - 方法1：完全跟隨燈光軌跡
        glm::mat4 mxRot = glm::rotate(glm::mat4(1.0f), currentAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 pos = glm::vec4(lightStartPos, 1.0f);
        glm::vec3 calculatedPos = glm::vec3(mxRot * pos);
        
        // 添加偏移避免重疊（可調整）
        glm::vec3 modelOffset = glm::vec3(0.0f, -2.0f, 0.0f); // Y軸向下偏移
        calculatedPos += modelOffset;
        
        _modelMatrix = glm::translate(glm::mat4(1.0f), calculatedPos);
        _modelMatrix = glm::rotate(_modelMatrix, currentAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        
        std::cout << "=== MODEL FOLLOWING LIGHT ===" << std::endl;
        std::cout << "Light clock: " << lightClock << " / 4.0" << std::endl;
        std::cout << "Sync angle (degrees): " << glm::degrees(currentAngle) << std::endl;
        std::cout << "Light radius: " << lightRadius << std::endl;
        std::cout << "Model position: (" << calculatedPos.x << ", " << calculatedPos.y << ", " << calculatedPos.z << ")" << std::endl;
        std::cout << "==============================" << std::endl;
    }
    else if (shouldMove) {
        // 修正原始模型旋轉，使其更合理
        _clock += dt;
        if (_clock >= 4.0f) { // 改為與燈光相同的周期
            _clock = 0.0f;
        }
        
        float angle = _clock * M_PI_2; // 改為與燈光相同的角度計算
        float radius = 10.0f;
        
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        
        _modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
        _modelMatrix = glm::rotate(_modelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        
        std::cout << "=== MODEL AUTO ROTATION ===" << std::endl;
        std::cout << "Model clock: " << _clock << " / 4.0" << std::endl;
        std::cout << "Model angle (degrees): " << glm::degrees(angle) << std::endl;
        std::cout << "Model radius: " << radius << std::endl;
        std::cout << "Model position: (" << x << ", 0.0, " << z << ")" << std::endl;
        std::cout << "============================" << std::endl;
    }
}
