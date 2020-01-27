#include "../pch.h"
#include "SpeedLimitComponent.h"

SpeedLimitComponent::SpeedLimitComponent(float maxSpeed_) {
	ID = "SpeedLimitComponent";
	maxSpeed = maxSpeed_;
	normalMaxSpeed = maxSpeed_;
}

SpeedLimitComponent::~SpeedLimitComponent() {

}
