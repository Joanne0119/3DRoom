// CButton.h
#pragma once

#include "CSprite2D.h"
#include <glm/glm.hpp>

class CButton : public CSprite2D {
private:
    float     _width, _height;     // 按鈕寬高
    bool      _bActive;            // 目前是否按下
    glm::vec4 _colorOff, _colorOn; // 兩種狀態顏色
    glm::vec2 _minXY, _maxXY;      // 範圍，用於滑鼠偵測
public:
    // width, height：按鈕尺寸（像素） colOff, colOn：非按下/按下時的顏色
    CButton(float width = 50.0f, float height = 50.0f,
        glm::vec4 colOff = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
        glm::vec4 colOn = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f));
    virtual ~CButton();
    void init(GLuint shaderProg);
    void setScreenPos(float sx, float sy); // 以螢幕座標(左下角0,0)設定中心位置
    bool isMouseOver(float mouseX, float mouseY) const;  // true:游標在按鈕上
    bool handleClick(float mouseX, float mouseY);  // true:在按鈕上按下
    virtual void draw() override;
    virtual void drawRaw() override;

};