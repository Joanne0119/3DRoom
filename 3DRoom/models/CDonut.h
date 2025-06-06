#pragma once
#include "CShape.h"

class CDonut : public CShape {
public:
    // majorRadius: 大半徑, minorRadius: 小半徑
    // segments: 環向分段, slices: 圓截面分段, pattern: 顏色選擇 (1~4)
    CDonut(float majorRadius = 0.5f, float minorRadius = 0.2f, GLuint segments = 15, GLuint slices = 15, GLuint pattern = 1);
    virtual ~CDonut();

    virtual void draw() override;
    virtual void drawRaw() override;
    virtual void reset() override;
    virtual void update(float dt) override;

protected:
    void generateDonut(float majorRadius, float minorRadius, GLuint segments, GLuint slices, GLuint pattern);
};