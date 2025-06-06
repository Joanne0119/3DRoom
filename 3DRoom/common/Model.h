#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "../models/CShape.h"
#include "CLight.h"
// 需要包含 tiny_obj_loader.h
//#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// 頂點結構
struct Vertex {
    float position[3];
    float normal[3];
    float texCoords[2];
    
    Vertex() = default;
    Vertex(float px, float py, float pz,
           float nx, float ny, float nz,
           float tx, float ty) {
        position[0] = px; position[1] = py; position[2] = pz;
        normal[0] = nx; normal[1] = ny; normal[2] = nz;
        texCoords[0] = tx; texCoords[1] = ty;
    }
};

// 材質結構
struct Material {
    std::string name;
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float shininess;
    float alpha;
    
    GLuint diffuseTexture;
    GLuint normalTexture;
    GLuint specularTexture;
    GLuint alphaTexture;
    
    std::string diffuseTexPath;
    std::string normalTexPath;
    std::string specularTexPath;
    std::string alphaTexPath;
    
    Material() : shininess(32.0f), alpha(1.0f), diffuseTexture(0), normalTexture(0), specularTexture(0), alphaTexture(0) {
        ambient[0] = ambient[1] = ambient[2] = 0.2f;
        diffuse[0] = diffuse[1] = diffuse[2] = 0.8f;
        specular[0] = specular[1] = specular[2] = 1.0f;
    }
};

// 網格結構
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    int materialIndex;
    
    GLuint VAO, VBO, EBO;
    
    Mesh() : materialIndex(-1), VAO(0), VBO(0), EBO(0) {}
};

// 主要的模型類別
class Model : public CShape {
private:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::string directory;
    
    // 載入紋理的輔助函數
    GLuint LoadTexture(const std::string& path);
    
    // 處理材質
    void ProcessMaterials(const std::vector<tinyobj::material_t>& objMaterials);
    
    // 處理網格
    void ProcessMesh(const tinyobj::attrib_t& attrib,
                     const tinyobj::shape_t& shape,
                     const std::vector<tinyobj::material_t>& objMaterials);
    
    // 設置網格的 OpenGL 緩衝區
    void SetupMesh(Mesh& mesh);
    
    // 從檔案路徑中提取目錄
    std::string GetDirectory(const std::string& filepath);
    
    bool  _bautoRotate = false;
    float _clock = 0.0f;
    glm::mat4 _modelMatrix = glm::mat4(1.0f);
    CLight* _followLight = nullptr; 

public:
    Model() = default;
    ~Model();
    
    // 載入模型
    bool LoadModel(const std::string& filepath);
    
    // 渲染模型
    void Render(GLuint shaderProgram);
    
    // 清理資源
    void Cleanup();
    
    // 取得材質數量
    size_t GetMaterialCount() const { return materials.size(); }
    
    // 取得網格數量
    size_t GetMeshCount() const { return meshes.size(); }
    
    // 取得特定材質
    const Material& GetMaterial(size_t index) const;
    
    // 檢查是否成功載入
    bool IsLoaded() const { return !meshes.empty(); }
    void setAutoRotate();
    void update(float dt);
    void setRotate(float angle, const glm::vec3& axis) {
        _modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
        std::cout << "Setting rotation: " << angle << " degrees" << std::endl;
    }
    glm::mat4 getModelMatrix() const { return _modelMatrix; }
    
    void setFollowLight(CLight* light) { _followLight = light; }
};

#endif // MODEL_H
