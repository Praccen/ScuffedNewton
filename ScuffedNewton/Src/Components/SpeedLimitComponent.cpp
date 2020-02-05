#include "../pch.h"
#include "SpeedLimitComponent.h"

std::string SpeedLimitComponent::ID = "SpeedLimitComponent";

SpeedLimitComponent::SpeedLimitComponent(float maxSpeed_) {
	maxSpeed = maxSpeed_;
	normalMaxSpeed = maxSpeed_;
}

SpeedLimitComponent::~SpeedLimitComponent() {

}
