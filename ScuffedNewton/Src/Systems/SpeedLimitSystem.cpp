#include "../pch.h"
#include "SpeedLimitSystem.h"

#include "../Components/Components.h"
#include "../DataTypes/Entity.h"

SpeedLimitSystem::SpeedLimitSystem() {
	requiredComponents["MovementComponent"] = true;
	requiredComponents["SpeedLimitComponent"] = true;
}

SpeedLimitSystem::~SpeedLimitSystem() {
}

void SpeedLimitSystem::update() {
	for (auto& e : entities) {
		MovementComponent* movement = e->getComponent<MovementComponent>();
		SpeedLimitComponent* speedLimit = e->getComponent<SpeedLimitComponent>();

		// Retain vertical speed
		const float ySpeed = movement->velocity.y;

		// Calculate horizontal speed
		glm::vec3 newVelocity = glm::vec3(movement->velocity.x, 0.0f, movement->velocity.z);
		const float horizontalSpeedSquared = glm::length2(newVelocity);

		float maxSpeed = speedLimit->maxSpeed;
		
		// Limit max speed
		if (horizontalSpeedSquared > maxSpeed * maxSpeed) {
			newVelocity = glm::normalize(newVelocity) * speedLimit->maxSpeed;
		}

		// Limit min speed
		if (horizontalSpeedSquared < 0.05f * 0.05f) {	// Retains minimum speed from old code
			newVelocity = glm::vec3(0.0f);
		}

		// Set new velocity while retaining vertical speed
		newVelocity.y = ySpeed;
		movement->velocity = newVelocity;
	}
}
