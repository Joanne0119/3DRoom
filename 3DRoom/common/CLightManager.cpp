//  CLightManager.cpp
#include "CLightManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

CLightManager::CLightManager() : shaderID(0) {
    lights.reserve(MAX_LIGHTS);
}

CLightManager::~CLightManager() {
    // 注意：這裡不刪除 CLight 物件，因為它們可能在其他地方被管理
    // 如果需要自動刪除，可以改用 unique_ptr
}

void CLightManager::addLight(CLight* light) {
    if (lights.size() < MAX_LIGHTS && light != nullptr) {
        lights.push_back(light);
    }
}

void CLightManager::removeLight(int index) {
    if (index >= 0 && index < lights.size()) {
        lights.erase(lights.begin() + index);
    }
}

void CLightManager::clearLights() {
    lights.clear();
}

void CLightManager::setShaderID(GLuint shaderProg) {
    shaderID = shaderProg;
    
    // 設定光源數量
    GLint loc = glGetUniformLocation(shaderID, "uNumLights");
    glUniform1i(loc, static_cast<int>(lights.size()));
    
    updateAllLightsToShader();
}

void CLightManager::updateAllLightsToShader() {
    if (shaderID == 0) return;
    
    // 更新光源數量
    GLint loc = glGetUniformLocation(shaderID, "uNumLights");
    glUniform1i(loc, static_cast<int>(lights.size()));
    
    // 更新每個光源的資料
    for (int i = 0; i < lights.size() && i < MAX_LIGHTS; i++) {
        CLight* light = lights[i];
        std::string lightName = "uLights[" + std::to_string(i) + "]";
        
        // Position
        loc = glGetUniformLocation(shaderID, (lightName + ".position").c_str());
        glm::vec3 pos = light->getPos();
        glUniform3fv(loc, 1, glm::value_ptr(pos));
        
        // Colors (考慮光源開關狀態)
        glm::vec4 amb = light->isLightOn() ? light->getAmbient() : glm::vec4(0.0f);
        glm::vec4 diff = light->isLightOn() ? light->getDiffuse() : glm::vec4(0.0f);
        glm::vec4 spec = light->isLightOn() ? light->getSpecular() : glm::vec4(0.0f);
        
        loc = glGetUniformLocation(shaderID, (lightName + ".ambient").c_str());
        glUniform4fv(loc, 1, glm::value_ptr(amb));
        
        loc = glGetUniformLocation(shaderID, (lightName + ".diffuse").c_str());
        glUniform4fv(loc, 1, glm::value_ptr(diff));
        
        loc = glGetUniformLocation(shaderID, (lightName + ".specular").c_str());
        glUniform4fv(loc, 1, glm::value_ptr(spec));
        
        // Attenuation
        float c, l, q;
        light->getAttenuation(c, l, q);
        
        loc = glGetUniformLocation(shaderID, (lightName + ".constant").c_str());
        glUniform1f(loc, c);
        
        loc = glGetUniformLocation(shaderID, (lightName + ".linear").c_str());
        glUniform1f(loc, l);
        
        loc = glGetUniformLocation(shaderID, (lightName + ".quadratic").c_str());
        glUniform1f(loc, q);
        
        // Light type and enabled state
        loc = glGetUniformLocation(shaderID, (lightName + ".type").c_str());
        glUniform1i(loc, static_cast<int>(light->getType()));
        
        loc = glGetUniformLocation(shaderID, (lightName + ".enabled").c_str());
        glUniform1i(loc, light->isLightOn() ? 1 : 0);
        
        // Spot light specific parameters
        if (light->getType() == CLight::LightType::SPOT) {
            loc = glGetUniformLocation(shaderID, (lightName + ".direction").c_str());
            glm::vec3 dir = light->getDirection();
            glUniform3fv(loc, 1, glm::value_ptr(dir));
            
            loc = glGetUniformLocation(shaderID, (lightName + ".cutOff").c_str());
            glUniform1f(loc, light->getInnerCutOff());
            
            loc = glGetUniformLocation(shaderID, (lightName + ".outerCutOff").c_str());
            glUniform1f(loc, light->getOuterCutOff());
            
            loc = glGetUniformLocation(shaderID, (lightName + ".exponent").c_str());
            glUniform1f(loc, light->getExponent());
        }
    }
}

void CLightManager::update(float dt) {
    for (auto& light : lights) {
        light->update(dt);
    }
}

void CLightManager::draw() {
    for (auto& light : lights) {
        light->draw();
    }
}

void CLightManager::drawRaw() {
    for (auto& light : lights) {
        light->drawRaw();
    }
}

CLight* CLightManager::getLight(int index) {
    if (index >= 0 && index < lights.size()) {
        return lights[index];
    }
    return nullptr;
}

