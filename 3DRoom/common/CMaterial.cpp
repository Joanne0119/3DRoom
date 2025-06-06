#include "CMaterial.h"
#include <glm/gtc/type_ptr.hpp>

CMaterial::CMaterial(glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float shininess) {
    _ambient = ambient; _diffuse = diffuse; _specular = specular; _shininess = shininess;
    _ambient.w = 1.0f; _diffuse.w = 1.0f; _specular.w = 1.0f; // 設定透明度為 1 (不透明)
}

CMaterial::~CMaterial() = default;

void CMaterial::setAmbient(glm::vec4 amb) { _ambient = amb; }
glm::vec4 CMaterial::getAmbient() { return _ambient; }

void CMaterial::setDiffuse(glm::vec4 diff) { _diffuse = diff; }
glm::vec4 CMaterial::getDiffuse() { return _diffuse; }

void CMaterial::setSpecular(glm::vec4 spec) { _specular = spec; }
glm::vec4 CMaterial::getSpecular() { return _specular; }

void CMaterial::setShininess(float shininess) { _shininess = shininess; }
float CMaterial::getShininess() { return _shininess; }

void CMaterial::uploadToShader(GLuint shaderProg, std::string name) {
    GLint loc = glGetUniformLocation(shaderProg, (name + ".ambient").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(_ambient));

    loc = glGetUniformLocation(shaderProg, (name + ".diffuse").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(_diffuse));

    loc = glGetUniformLocation(shaderProg, (name + ".specular").c_str());
    glUniform4fv(loc, 1, glm::value_ptr(_specular));

    loc = glGetUniformLocation(shaderProg, (name + ".shininess").c_str());
    glUniform1f(loc, _shininess);
}