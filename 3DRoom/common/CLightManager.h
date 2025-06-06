//  CLightManager.h

#pragma once

#include "CLight.h"
#include <vector>
#include <GL/glew.h>

#define MAX_LIGHTS 8

class CLightManager {
private:
    std::vector<CLight*> lights;
    GLuint shaderID;
    
public:
    CLightManager();
    ~CLightManager();
    
    // 光源管理
    void addLight(CLight* light);
    void removeLight(int index);
    void clearLights();
    
    // Shader 設定
    void setShaderID(GLuint shaderProg);
    void updateAllLightsToShader();
    
    // 更新和繪製
    void update(float dt);
    void draw();
    void drawRaw();
    
    // 取得光源數量
    int getLightCount() const { return static_cast<int>(lights.size()); }
    CLight* getLight(int index);
};

