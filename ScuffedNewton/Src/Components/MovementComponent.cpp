#include "../pch.h"
#include "MovementComponent.h"

std::string MovementComponent::ID = "MovementComponent";

MovementComponent::MovementComponent() {
	velocity = glm::vec3(0.0f);
	relVel = glm::vec3(0.0f);
	rotation = glm::vec3(0.0f);
	constantAcceleration = glm::vec3(0.0f);
	accelerationToAdd = glm::vec3(0.0f);

	oldVelocity = glm::vec3(0.0f);
	oldMovement = glm::vec3(0.0f);

	airDrag = 1.0f;

	updateableDt = 0.0f;
}

MovementComponent::~MovementComponent() {

}
