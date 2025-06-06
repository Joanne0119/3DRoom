#pragma once
#include "CShape.h"

class CQuad : public CShape
{
public:
	CQuad();
	virtual ~CQuad();
	virtual void draw() override;
	virtual void drawRaw() override;
	virtual void reset() override;
	virtual void update(float dt) override;
};