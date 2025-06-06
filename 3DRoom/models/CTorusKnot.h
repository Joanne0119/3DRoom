#pragma once
#include "CShape.h"

class CTorusKnot : public CShape {
public:
    // p, q 決定環的纏繞次數，radius 為主半徑，tube 為小圓半徑
    CTorusKnot(unsigned int p = 2, unsigned int q = 3,
               float radius = 1.0f, float tube = 0.2f,
               unsigned int segments = 200, unsigned int sides = 16,
               GLuint pattern = 1);
    virtual ~CTorusKnot();

    virtual void draw() override;
    virtual void drawRaw() override;
    virtual void update(float dt) override;

protected:
    void generateTorusKnot(unsigned int p, unsigned int q,
                           float radius, float tube,
                           unsigned int segments, unsigned int sides,
                           GLuint pattern);
};
