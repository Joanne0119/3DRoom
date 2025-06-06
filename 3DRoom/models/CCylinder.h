// CCylinder.h
#pragma once

#include "CShape.h"

class CCylinder : public CShape {
public:
    // sectors: 圓周分段數(預設 36)，pattern: 顏色選擇 (1–4)
    CCylinder(GLuint sectors = 36, GLuint pattern = 1);
    virtual ~CCylinder();

    virtual void draw() override;
    virtual void drawRaw() override;
    virtual void reset() override;
    virtual void update(float dt) override;

private:
    void generateCylinder(GLuint sectors, GLuint pattern);
};
