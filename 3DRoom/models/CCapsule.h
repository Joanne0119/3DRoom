// CCapsule.h
#pragma once

#include "CShape.h"

class CCapsule : public CShape {
public:
    // radius: 半球半徑 (預設 0.5)
    // height: 圓柱高度 (預設 1.0)
    // sectors: 圓周分段 (預設 36)
    // stacks: 半球分段 (預設 18)
    // pattern: 顏色選擇 (1–4)
    CCapsule(float radius = 0.5f, float height = 1.0f, GLuint sectors = 36, GLuint stacks = 18, GLuint pattern = 1);
    virtual ~CCapsule();

    virtual void draw() override;
    virtual void drawRaw() override;
    virtual void reset() override;
    virtual void update(float dt) override;

protected:
    void generateCapsule(float radius,  float height, GLuint sectors,  GLuint stacks,  GLuint pattern);
};
