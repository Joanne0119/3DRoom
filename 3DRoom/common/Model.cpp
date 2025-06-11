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
        
        if (!objMat.alpha_texname.empty()) {
            mat.alphaTexPath = directory + "/" + objMat.alpha_texname;
            mat.alphaTexture = LoadTexture(mat.alphaTexPath);
        }
        
        std::vector<std::string> lightMapExtensions = {"_lightmap.png", "_lightmap.jpg"};
        for (const std::string& ext : lightMapExtensions) {
            std::string lightMapPath = directory + "/" + mat.name + ext;
            
            // 檢查檔案是否真的存在且可讀
            std::ifstream testFile(lightMapPath, std::ios::binary);
            if (testFile.good() && testFile.is_open()) {
                testFile.close();
                
                // 嘗試載入紋理
                GLuint testTexture = LoadTexture(lightMapPath);
                if (testTexture != 0 && glIsTexture(testTexture)) {
                    mat.lightMapTexPath = lightMapPath;
                    mat.lightMapTexture = testTexture;
                    mat.hasLightMap = true;
                    mat.lightMapIntensity = 1.0f;
                    std::cout << "  Auto-detected light map: " << lightMapPath
                              << " (ID: " << testTexture << ")" << std::endl;
                    break;
                } else {
                    std::cout << "  Light map file exists but failed to load: " << lightMapPath << std::endl;
                    if (testTexture != 0) {
                        glDeleteTextures(1, &testTexture);
                    }
                }
            }
        }
        
        // 如果沒有找到 Light Map，確保相關變數是正確的
        if (!mat.hasLightMap) {
            mat.lightMapTexture = 0;
            mat.lightMapTexPath = "";
            mat.lightMapIntensity = 1.0f;
            std::cout << "  No light map found for material: " << mat.name << std::endl;
        }
        
        if (!mat.hasEnvironmentMap) {
            std::vector<std::string> envMapPatterns = {
                mat.name + "_env",
                mat.name + "_environment",
                mat.name + "_cubemap",
                mat.name + "_skybox"
            };
            
            for (const auto& pattern : envMapPatterns) {
                std::string basePath = directory + "/" + pattern;
                
                // 首先嘗試六面 Cube Map
                GLuint cubeMapTexture = LoadCubeMapFromFiles(basePath);
                if (cubeMapTexture != 0) {
                    mat.environmentMapTexture = cubeMapTexture;
                    mat.environmentMapPath = basePath;
                    mat.hasEnvironmentMap = true;
                    mat.reflectivity = 0.3f; // 預設反射率
                    std::cout << "  Found 6-face cube map: " << basePath << std::endl;
                    break;
                }
                
                // 如果沒找到六面，嘗試單一檔案
                std::vector<std::string> extensions = {".png", ".jpg", ".hdr", ".tga"};
                for (const auto& ext : extensions) {
                    std::string singlePath = basePath + ext;
                    std::ifstream testFile(singlePath);
                    if (testFile.good()) {
                        testFile.close();
                        
                        GLuint envTexture = 0;
                       
                        envTexture = LoadCubeMapFromSingleImage(singlePath);
                        
                        if (envTexture != 0) {
                            mat.environmentMapTexture = envTexture;
                            mat.environmentMapPath = singlePath;
                            mat.hasEnvironmentMap = true;
                            mat.reflectivity = 0.3f;
                            std::cout << "  Found single environment map: " << singlePath << std::endl;
                            break;
                        }
                    }
                }
                if (mat.hasEnvironmentMap) break;
            }
        }
        
        // 如果都沒找到，確保變數正確初始化
        if (!mat.hasEnvironmentMap) {
            mat.environmentMapTexture = 0;
            mat.environmentMapPath = "";
            mat.reflectivity = 0.0f;
            std::cout << "  No environment map found for material: " << mat.name << std::endl;
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
    std::cout << "  attrib.normals size: " << attrib.normals.size() << std::endl;
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
        
        if (path.find("alpha") != std::string::npos) {
            std::cout << "Loading alpha texture: " << path << std::endl;
            std::cout << "Alpha texture size: " << width << "x" << height
                      << ", components: " << nrComponents << std::endl;
        }
        
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
    
    std::vector<size_t> opaqueMeshes;
    std::vector<size_t> transparentMeshes;
    
    for (size_t i = 0; i < meshes.size(); i++) {
        const Mesh& mesh = meshes[i];
        bool isTransparent = false;
        
        if (mesh.materialIndex >= 0 && mesh.materialIndex < materials.size()) {
            const Material& material = materials[mesh.materialIndex];
            isTransparent = (material.alpha < 1.0f) || (material.alphaTexture != 0);
        }
        
        if (isTransparent) {
            transparentMeshes.push_back(i);
        } else {
            opaqueMeshes.push_back(i);
        }
    }
    // 如果有透明物體，需要啟用混合
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    
    for (size_t i : opaqueMeshes) {
        RenderMesh(i, shaderProgram);
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);  // 禁止寫入深度緩衝區，但仍進行深度測試
    
    for (size_t i : transparentMeshes) {
        RenderMesh(i, shaderProgram);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Model::RenderMesh(size_t meshIndex, GLuint shaderProgram) {
    const Mesh& mesh = meshes[meshIndex];
    
    // 重置紋理單元
    for (int i = 0; i < 6; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    
    // 設置預設值
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasDiffuseTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasNormalTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasSpecularTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasAlphaTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasLightMap"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasEnvironmentMap"), 0);
    glUniform1f(glGetUniformLocation(shaderProgram, "uMaterial.alpha"), 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uMaterial.lightMapIntensity"), 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uMaterial.reflectivity"), 0.0f);
    
        // 綁定材質
        if (mesh.materialIndex >= 0 && mesh.materialIndex < materials.size()) {
            const Material& material = materials[mesh.materialIndex];
            std::cout << "EnvMap: " << material.environmentMapTexture << ", hasEnv: " << material.hasEnvironmentMap << std::endl;

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
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasAlphaTexture"), 1);  // 添加這行！
                std::cout << "    Using alpha texture" << std::endl;
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasAlphaTexture"), 0);  // 添加這行！
                std::cout << "    No alpha texture" << std::endl;
            }
            if (material.lightMapTexture != 0) {
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, material.lightMapTexture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.lightMapTexture"), 4);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasLightMap"), 1);
                glUniform1f(glGetUniformLocation(shaderProgram, "uMaterial.lightMapIntensity"), material.lightMapIntensity);
                
               glUniform1f(glGetUniformLocation(shaderProgram, "uLightMapGamma"), 1.0f);
               glUniform1i(glGetUniformLocation(shaderProgram, "uLightMapBlendMode"), 1);  // 0=Multiply
               glUniform1i(glGetUniformLocation(shaderProgram, "uUseLightMapAO"), 0);      // 或 1 如果要用作 AO
                std::cout << "    Using light map texture (ID: " << material.lightMapTexture << ")" << std::endl;
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasLightMap"), 0);
            }
            
            if (material.environmentMapTexture != 0) {
                glActiveTexture(GL_TEXTURE5);
                
                GLint boundTexture;
                glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &boundTexture);
                std::cout << "    Bound cube map texture ID: " << boundTexture << std::endl;
                glBindTexture(GL_TEXTURE_CUBE_MAP, material.environmentMapTexture);
                
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.environmentMap"), 5);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasEnvironmentMap"), 1);
                glUniform1f(glGetUniformLocation(shaderProgram, "uMaterial.reflectivity"), material.reflectivity); // 傳遞反射強度
                
                std::cout << "    Using environment map texture (ID: " << material.environmentMapTexture
                                      << "), Reflectivity: " << material.reflectivity << std::endl;
            } else {
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                glUniform1i(glGetUniformLocation(shaderProgram, "uMaterial.hasEnvironmentMap"), 0);
            }

        } else {
            std::cout << "  Mesh has no material assigned" << std::endl;
        }
    
    // 渲染
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // 檢查 OpenGL 錯誤
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "  OpenGL error during rendering: " << error << std::endl;
    }
    
    // 清理紋理綁定
    for (int i = 0; i < 6; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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
        if (material.lightMapTexture != 0) glDeleteTextures(1, &material.lightMapTexture);
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

void Model::update(float dt) {
    updateCameraFollow();
    
    // 原有的移動和旋轉邏輯...
    float _boundaryLeft = -20.0f;
    float _boundaryRight = 20.0f;
    float _boundaryTop = 20.0f;
    float _boundaryBottom = -20.0f;
    bool shouldMove = _bautoRotate;
    
    if (shouldMove) {
        // 移動邏輯保持不變...
        glm::vec3 delta = _direction * _speed * dt;
        _position += delta;

        // 邊界檢測和轉向邏輯...
        if (_position.x > _boundaryRight && _direction.x > 0) {
            _position.x = _boundaryRight;
            _direction = glm::vec3(0.0f, 0.0f, -1.0f);
            _targetAngle = glm::radians(180.0f);
        }
        else if (_position.z < _boundaryBottom && _direction.z < 0) {
            _position.z = _boundaryBottom;
            _direction = glm::vec3(-1.0f, 0.0f, 0.0f);
            _targetAngle = glm::radians(270.0f);
        }
        else if (_position.x < _boundaryLeft && _direction.x < 0) {
            _position.x = _boundaryLeft;
            _direction = glm::vec3(0.0f, 0.0f, 1.0f);
            _targetAngle = glm::radians(0.0f);
        }
        else if (_position.z > _boundaryTop && _direction.z > 0) {
            _position.z = _boundaryTop;
            _direction = glm::vec3(1.0f, 0.0f, 0.0f);
            _targetAngle = glm::radians(90.0f);
        }
        
        // 如果不是 Billboard，才進行正常的角度插值
        if (!_isBillboard) {
            float angleDiff = _targetAngle - _currentAngle;
            if (angleDiff > glm::pi<float>()) angleDiff -= glm::two_pi<float>();
            if (angleDiff < -glm::pi<float>()) angleDiff += glm::two_pi<float>();

            float maxStep = _rotationSpeed * dt;
            if (glm::abs(angleDiff) < maxStep) {
                _currentAngle = _targetAngle;
            } else {
                _currentAngle += glm::sign(angleDiff) * maxStep;
            }
        }
    }
    
    // 計算模型矩陣
    if (_isBillboard) {
        // 使用 Billboard 計算
        _modelMatrix = calculateBillboardMatrix(_cameraPos, _viewMatrix);
    } else {
        // 正常的變換計算
        if (shouldMove || !_bSelfRotate) {
            _modelMatrix = glm::translate(glm::mat4(1.0f), _position);
            if (!_isBillboard) {
                _modelMatrix = glm::rotate(_modelMatrix, _currentAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }
        
        // 自轉
        if (_bSelfRotate) {
            _selfRotationAngle += _selfRotationSpeed * dt;
            if (_selfRotationAngle > glm::two_pi<float>()) {
                _selfRotationAngle -= glm::two_pi<float>();
            }
            if (!shouldMove) {
                _modelMatrix = glm::translate(glm::mat4(1.0f), _position);
            }
            _modelMatrix = glm::rotate(_modelMatrix, _selfRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }
}


void Model::setFollowCamera(bool follow, const glm::vec3& offset, bool followRotation = true, float rotationOffset = 0.0f) {
    _followCamera = follow;
    _cameraOffset = offset;
    _followCameraRotation = followRotation;
    _rotationOffset = rotationOffset;
    
    if (follow) {
        std::cout << "Model now following camera with offset: ("
                  << offset.x << ", " << offset.y << ", " << offset.z << ")";
        if (followRotation) {
            std::cout << " and rotation with offset: " << glm::degrees(rotationOffset) << " degrees";
        }
        std::cout << std::endl;
    } else {
        std::cout << "Model stopped following camera" << std::endl;
    }
}

void Model::updateCameraFollow() {
    if (!_followCamera) return;
    
    // 獲取攝影機位置
    glm::vec3 cameraPos = _cameraPos;
    
    // 獲取攝影機的前方、右方、上方向量
    glm::mat4 viewMatrix = _viewMatrix;
    glm::vec3 cameraForward = -glm::vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]);
    glm::vec3 cameraRight = glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
    glm::vec3 cameraUp = glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
    
    // 計算物件應該在的位置
    // offset.x = 右方偏移, offset.y = 上方偏移, offset.z = 前方偏移
    glm::vec3 targetPosition = cameraPos +
                              cameraRight * _cameraOffset.x +
                              cameraUp * _cameraOffset.y +
                              cameraForward * _cameraOffset.z;
    
    // 更新物件位置
    _position = targetPosition;
    
    float targetAngle = _currentAngle;
    
    if (_followCameraRotation) {
            // 方法1：根據攝影機前方向量計算 Y 軸旋轉角度
            targetAngle = atan2(cameraForward.x, cameraForward.z) + _rotationOffset;
            
            // 或者使用方法2：更精確的四元數轉換
            // glm::quat cameraRotation = glm::conjugate(glm::quat_cast(viewMatrix));
            // glm::vec3 eulerAngles = glm::eulerAngles(cameraRotation);
            // targetAngle = eulerAngles.y + _rotationOffset;
        }
    
    // 更新模型矩陣 - 只有位移，保持原有的旋轉
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), _position);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), targetAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    _modelMatrix = translation * rotation;
    
    if (_followCameraRotation) {
        _currentAngle = targetAngle;
    }
    
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) {
        std::cout << "Following model - Position: (" << _position.x << ", " << _position.y << ", " << _position.z << ")";
        if (_followCameraRotation) {
            std::cout << ", Rotation: " << glm::degrees(targetAngle) << " degrees";
        }
        std::cout << std::endl;
    }
}

void Model::setCameraPos(const glm::vec3& pos) {
    _cameraPos = pos;
     std::cout << "Model received camera pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
}

void Model::setViewMatrix(const glm::mat4& viewMatrix) {
    _viewMatrix = viewMatrix;
    // Debug 輸出 - 只打印一次避免過多輸出
    static int debugCount = 0;
    if (debugCount < 5) {
        std::cout << "Model received view matrix update " << debugCount << std::endl;
        debugCount++;
    }
}

void Model::SetLightMap(const std::string& materialName, const std::string& lightMapPath, float intensity = 1.0f) {
    for (auto& mat : materials) {
        if (mat.name == materialName) {
            mat.lightMapTexPath = lightMapPath;
            mat.lightMapTexture = LoadTexture(lightMapPath);
            mat.hasLightMap = (mat.lightMapTexture != 0);
            mat.lightMapIntensity = intensity;
            std::cout << "Set light map for material '" << materialName << "': " << lightMapPath << std::endl;
            break;
        }
    }
}

void Model::SetEnvironmentMap(const std::string& materialName, const std::string& environmentMapPath, float reflectivity) {
    for (auto& mat : materials) {
        if (mat.name == materialName) {
            mat.environmentMapPath = environmentMapPath;
            mat.environmentMapTexture = LoadCubeMapFromSingleImage(environmentMapPath);
            mat.hasEnvironmentMap = (mat.environmentMapTexture != 0);
            mat.reflectivity = reflectivity;
            
            // 確保反射率在合理範圍內
            if (mat.reflectivity < 0.0f) mat.reflectivity = 0.0f;
            if (mat.reflectivity > 1.0f) mat.reflectivity = 1.0f;
            
            std::cout << "Set environment map for material '" << materialName
                      << "': " << environmentMapPath
                      << " (Texture ID: " << mat.environmentMapTexture
                      << ", Reflectivity: " << mat.reflectivity << ")" << std::endl;
            break;
        }
    }
}

void Model::SetEnvironmentMapFromFiles(const std::string& materialName, const std::string& environmentMapPath, float reflectivity) {
    for (auto& mat : materials) {
        if (mat.name == materialName) {
            mat.environmentMapPath = environmentMapPath;
            mat.environmentMapTexture = LoadCubeMapFromFiles(environmentMapPath);
            mat.hasEnvironmentMap = (mat.environmentMapTexture != 0);
            mat.reflectivity = reflectivity;
            
            // 確保反射率在合理範圍內
            if (mat.reflectivity < 0.0f) mat.reflectivity = 0.0f;
            if (mat.reflectivity > 1.0f) mat.reflectivity = 1.0f;
            
            std::cout << "Set environment map for material '" << materialName
                      << "': " << environmentMapPath
                      << " (Texture ID: " << mat.environmentMapTexture
                      << ", Reflectivity: " << mat.reflectivity << ")" << std::endl;
            break;
        }
    }
}

GLuint Model::LoadCubeMapFromSingleImage(const std::string& path) {
    // 檢查檔案是否存在
    std::ifstream file(path);
    if (!file) {
        std::cout << "Cube map file not found: " << path << std::endl;
        return 0;
    }
    file.close();
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    stbi_set_flip_vertically_on_load(false); // Cube map 不需要翻轉
    
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    
    if (data) {
        GLenum format;
        if (nrComponents == 3) {
            format = GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
        } else {
            std::cout << "Unsupported cube map format: " << nrComponents << " components" << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        
        // 將同一張圖片用於立方體的所有六個面
        for (unsigned int i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            
            // 檢查每個面是否正確上傳
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cout << "OpenGL error uploading cube map face " << i << ": " << error << std::endl;
            }
        }
        
        // 設置紋理參數 - 這些參數很重要！
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        // 檢查紋理參數設置是否有錯誤
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "OpenGL error setting cube map parameters: " << error << std::endl;
        }
        
        // 檢查紋理是否完整
        if (glIsTexture(textureID)) {
            std::cout << "Successfully created cube map texture (ID: " << textureID << ")" << std::endl;
        } else {
            std::cout << "Failed to create valid cube map texture" << std::endl;
        }
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // 解綁
        stbi_image_free(data);
        
        std::cout << "Successfully loaded cube map from single image: " << path
                  << " (Size: " << width << "x" << height << ", Components: " << nrComponents << ")" << std::endl;
        
        return textureID;
    } else {
        std::cout << "Failed to load cube map image: " << path << std::endl;
        std::cout << "STB error: " << stbi_failure_reason() << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }
}

GLuint Model::LoadCubeMapFromFiles(const std::string& basePath) {
    // 標準 OpenGL cubemap 面順序和對應的檔案名稱
    // 注意：這裡的命名對應標準的 OpenGL 座標系
    std::vector<std::pair<std::string, GLenum>> faces = {
        {"right",  GL_TEXTURE_CUBE_MAP_POSITIVE_X}, // +X (右)
        {"left",   GL_TEXTURE_CUBE_MAP_NEGATIVE_X}, // -X (左)
        {"top",    GL_TEXTURE_CUBE_MAP_POSITIVE_Y}, // +Y (上)
        {"bottom", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y}, // -Y (下)
        {"front",  GL_TEXTURE_CUBE_MAP_POSITIVE_Z}, // +Z (前)
        {"back",   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z}  // -Z (後)
    };
    
    // 支援的擴展名
    std::vector<std::string> extensions = {".jpg", ".png", ".tga", ".bmp", ".hdr"};
    
    std::vector<std::string> facePaths;
    
    // 找到所有六個面的檔案
    for (const auto& face : faces) {
        std::string foundPath = "";
        const std::string& faceName = face.first;
        
        // 嘗試不同的命名格式
        std::vector<std::string> namingPatterns = {
            basePath + "_" + faceName,              // skybox_right.jpg
            basePath + "/" + faceName,              // skybox/right.jpg
            basePath + "_" + faceName.substr(0,1),  // skybox_r.jpg
            basePath + "_" + std::to_string(&face - &faces[0]), // skybox_0.jpg (索引)
        };
        
        // 嘗試常見的替代命名
        if (faceName == "right") namingPatterns.push_back(basePath + "_px");
        if (faceName == "left")  namingPatterns.push_back(basePath + "_nx");
        if (faceName == "top")   namingPatterns.push_back(basePath + "_py");
        if (faceName == "bottom") namingPatterns.push_back(basePath + "_ny");
        if (faceName == "front") namingPatterns.push_back(basePath + "_pz");
        if (faceName == "back")  namingPatterns.push_back(basePath + "_nz");
        
        for (const auto& pattern : namingPatterns) {
            for (const auto& ext : extensions) {
                std::string testPath = pattern + ext;
                std::ifstream file(testPath);
                if (file.good()) {
                    file.close();
                    foundPath = testPath;
                    break;
                }
            }
            if (!foundPath.empty()) break;
        }
        
        if (foundPath.empty()) {
            std::cout << "Cube map face not found for: " << faceName << std::endl;
            return 0;
        }
        
        facePaths.push_back(foundPath);
        std::cout << "Found cube map face: " << foundPath << std::endl;
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    // 啟用無縫 cubemap（重要！）
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    // Cubemap 通常不需要垂直翻轉，但根據來源可能需要
    stbi_set_flip_vertically_on_load(false);
    
    // 載入每個面
    for (size_t i = 0; i < faces.size(); i++) {
        int width, height, nrComponents;
        unsigned char* data = stbi_load(facePaths[i].c_str(), &width, &height, &nrComponents, 0);
        
        if (data) {
            // 檢查所有面是否為正方形且尺寸相同
            if (width != height) {
                std::cout << "Warning: Cubemap face " << faces[i].first
                         << " is not square (" << width << "x" << height << ")" << std::endl;
            }
            
            GLenum format, internalFormat;
            
            switch (nrComponents) {
                case 1:
                    format = GL_RED;
                    internalFormat = GL_R8;
                    break;
                case 3:
                    format = GL_RGB;
                    internalFormat = GL_RGB8;
                    break;
                case 4:
                    format = GL_RGBA;
                    internalFormat = GL_RGBA8;
                    break;
                default:
                    std::cout << "Unsupported format for face " << faces[i].first
                              << ": " << nrComponents << " components" << std::endl;
                    stbi_image_free(data);
                    glDeleteTextures(1, &textureID);
                    return 0;
            }
            
            // 直接使用對應的 OpenGL 面目標
            glTexImage2D(faces[i].second, 0, internalFormat,
                        width, height, 0, format, GL_UNSIGNED_BYTE, data);
            
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cout << "OpenGL error uploading cube map face " << faces[i].first
                          << ": " << error << std::endl;
                stbi_image_free(data);
                glDeleteTextures(1, &textureID);
                return 0;
            }
            
            std::cout << "  Loaded face " << faces[i].first << ": "
                      << width << "x" << height << ", " << nrComponents << " components" << std::endl;
            
            stbi_image_free(data);
        } else {
            std::cout << "Failed to load cube map face: " << facePaths[i] << std::endl;
            std::cout << "STB error: " << stbi_failure_reason() << std::endl;
            glDeleteTextures(1, &textureID);
            return 0;
        }
    }
    
    // 設置紋理參數 - 對 cubemap 很重要
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    // 生成 mipmap - 對環境貼圖品質很重要
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL error setting cube map parameters: " << error << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    std::cout << "Successfully loaded cube map (ID: " << textureID << ")" << std::endl;
    return textureID;
}

void Model::setBillboard(bool enable, BillboardType type) {
    _isBillboard = enable;
    _billboardType = type;
    
    if (enable) {
        std::cout << "Billboard enabled with type: ";
        switch (type) {
            case BillboardType::SPHERICAL:
                std::cout << "SPHERICAL";
                break;
            case BillboardType::CYLINDRICAL:
                std::cout << "CYLINDRICAL";
                break;
            case BillboardType::SCREEN_ALIGNED:
                std::cout << "SCREEN_ALIGNED";
                break;
        }
        std::cout << std::endl;
    } else {
        std::cout << "Billboard disabled" << std::endl;
    }
}

void Model::setBillboardUpVector(const glm::vec3& up) {
    _billboardUp = glm::normalize(up);
}

glm::mat4 Model::calculateBillboardMatrix(const glm::vec3& cameraPos, const glm::mat4& viewMatrix) {
    switch (_billboardType) {
        case BillboardType::SPHERICAL:
            return calculateSphericalBillboard(cameraPos);
        case BillboardType::CYLINDRICAL:
            return calculateCylindricalBillboard(cameraPos);
        case BillboardType::SCREEN_ALIGNED:
            return calculateScreenAlignedBillboard(viewMatrix);
        default:
            return calculateSphericalBillboard(cameraPos);
    }
}

glm::mat4 Model::calculateSphericalBillboard(const glm::vec3& cameraPos) {
    // 計算從物體到攝影機的向量（物體的新前方向）
    glm::vec3 forward = glm::normalize(cameraPos - _position);
    
    // 使用世界座標的上方向量作為參考
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // 計算右方向量
    glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
    
    // 重新計算上方向量（確保正交）
    glm::vec3 up = glm::cross(forward, right);
    
    // 建立旋轉矩陣
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);      // X軸
    rotation[1] = glm::vec4(up, 0.0f);         // Y軸
    rotation[2] = glm::vec4(forward, 0.0f);    // Z軸
    
    // 結合位移和旋轉
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), _position);
    
    return translation * rotation;
}

glm::mat4 Model::calculateCylindricalBillboard(const glm::vec3& cameraPos) {
    // 計算攝影機在XZ平面上的投影位置
    glm::vec3 cameraXZ = glm::vec3(cameraPos.x, _position.y, cameraPos.z);
    
    // 計算從物體到攝影機在XZ平面的方向
    glm::vec3 forward = glm::normalize(cameraXZ - _position);
    
    // 使用固定的上方向量
    glm::vec3 up = _billboardUp;
    
    // 計算右方向量
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    
    // 重新正交化前方向量
    forward = glm::cross(right, up);
    
    // 建立旋轉矩陣
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(forward, 0.0f);
    
    // 結合位移和旋轉
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), _position);
    
    return translation * rotation;
}

glm::mat4 Model::calculateScreenAlignedBillboard(const glm::mat4& viewMatrix) {
    // 從視圖矩陣中提取攝影機的座標軸
    glm::vec3 right = glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
    glm::vec3 up = glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
    glm::vec3 forward = -glm::vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]);
    
    // 建立旋轉矩陣（直接使用攝影機的座標軸）
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(forward, 0.0f);
    
    // 結合位移和旋轉
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), _position);
    
    return translation * rotation;
}
