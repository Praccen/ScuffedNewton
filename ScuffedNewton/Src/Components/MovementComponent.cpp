#include "../pch.h"
#include "MovementComponent.h"

namespace Scuffed {

	std::string MovementComponent::ID = "MovementComponent";

	MovementComponent::MovementComponent() {
		velocity = glm::vec3(0.0f);
		relVel = glm::vec3(0.0f);
		rotation = glm::quat({ 0.f, 0.f, 0.f });
		constantAcceleration = glm::vec3(0.f);
		accelerationToAdd = glm::vec3(0.0f);

		oldVelocity = glm::vec3(0.0f);
		oldMovement = glm::vec3(0.0f);

		airDrag = 1.0f;

		updateableDt = 0.0f;
	}

	MovementComponent::~MovementComponent() {

	}

}