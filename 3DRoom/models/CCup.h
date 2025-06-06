// CCup.h
#pragma once

#include "CShape.h"

class CCup : public CShape {
public:
    // bottomR: 杯底半徑 (預設 0.3)
    // topR:   杯口半徑 (預設 0.5)
    // height: 杯高     (預設 1.0)
    // radialSegs: 環向分段 (預設 36)
    // heightSegs: 縱向分段 (預設 1)
    CCup(float bottomR = 0.3f, float topR = 0.5f, float height = 1.0f,
        unsigned int radialSegs = 36, unsigned int heightSegs = 1, GLuint pattern = 1);
    virtual ~CCup();

    virtual void draw()    override;
    virtual void drawRaw() override;
    virtual void update(float dt) override;

protected:
    // 生成杯身及杯底
    void generateCup(float bottomR, float topR, float height,
        unsigned int radialSegs, unsigned int heightSegs, GLuint pattern);
};
