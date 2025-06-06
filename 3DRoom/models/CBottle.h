#pragma once
#include "CShape.h"
#include <vector>

class CBottle : public CShape {
public:
    // radialSegs: 環向分段數
    // heightSegs: 高度方向分段數
    // pattern: 預設 1，提供 1~4 種顏色選擇
    CBottle(unsigned int radialSegs = 36, unsigned int heightSegs = 8, GLuint pattern = 1);
    virtual ~CBottle();

    virtual void draw() override;
    virtual void drawRaw() override;
    virtual void update(float dt) override;

protected:
    // 產生瓶身與底面，模型預設方向為 Z 軸朝上
    void generateBottle(unsigned int radialSegs, unsigned int heightSegs, GLuint pattern);
    std::vector<glm::vec2> bottleProfile = {
        {0.50f, 0.00f}, {0.48f, 0.05f}, {0.45f, 0.15f}, {0.40f, 0.40f},
        {0.35f, 0.80f}, {0.38f, 1.20f}, {0.45f, 1.60f}, {0.50f, 2.00f}};
};