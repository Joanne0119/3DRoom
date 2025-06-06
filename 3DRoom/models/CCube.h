#pragma once
#include "CShape.h"
#include "../common/CMaterial.h"

class CCube : public CShape
{
private:
	float _clock;
	bool  _bautoRotate;

public:
	CCube();
	virtual ~CCube();
	virtual void draw() override;
	virtual void drawRaw() override;
	virtual void reset() override;
	virtual void update(float dt) override;
	void setAutoRotate();
};
