#pragma once
#include "BaseSystem.h"

class SpeedLimitSystem final : public BaseSystem {
public:
	SpeedLimitSystem();
	~SpeedLimitSystem();

	void update();
};