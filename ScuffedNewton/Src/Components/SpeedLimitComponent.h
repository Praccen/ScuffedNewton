#pragma once
#include "Component.h"

class SpeedLimitComponent final : public Component {
public:
	SpeedLimitComponent(float maxSpeed_ = 9999999.0f)
		: maxSpeed(maxSpeed_)
		, normalMaxSpeed(maxSpeed_) {}
	~SpeedLimitComponent() {}

	float maxSpeed;
	float normalMaxSpeed;
};