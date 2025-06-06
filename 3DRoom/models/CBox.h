#pragma once
#include "CShape.h"
#include "../common/CMaterial.h"

class CBox : public CShape
{
private:

public:
	CBox();
	virtual ~CBox();
	virtual void draw() override;
	virtual void drawRaw() override;
	virtual void reset() override;
	virtual void update(float dt) override;
	void setAutoRotate();
};
