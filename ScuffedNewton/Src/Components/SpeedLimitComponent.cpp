#include "../pch.h"
#include "SpeedLimitComponent.h"

namespace Scuffed {

	std::string SpeedLimitComponent::ID = "SpeedLimitComponent";

	SpeedLimitComponent::SpeedLimitComponent(float maxSpeed_) {
		maxSpeed = maxSpeed_;
		normalMaxSpeed = maxSpeed_;
	}

	SpeedLimitComponent::~SpeedLimitComponent() {

	}

}