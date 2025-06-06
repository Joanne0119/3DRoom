// CTeapot.h
#pragma once

#include "CShape.h"

class CTeapot : public CShape {
public:
    // resolution：每個 patch 在 u、v 方向的細分段數，預設 10
    CTeapot(GLuint resolution = 5, GLuint pattern = 1);
    virtual ~CTeapot();

    virtual void draw() override;
    virtual void drawRaw() override;
    virtual void reset() override;
    virtual void update(float dt) override;

private:
    void generateTeapot(unsigned int resolution, GLuint pattern);

    // 以下兩個陣列請完整從 freeglut 的 fg_teapot_data.h 複製
    // Patch 索引（32 patches × 4 × 4 控制點）
    static const GLushort teapotPatches[32][4][4];
    // 控制點座標（306 個點 × 3 維）
    static const GLfloat teapotVertices[306][3];
};
