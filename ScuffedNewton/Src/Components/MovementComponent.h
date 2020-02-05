#pragma once
#include "Component.h"
#include <glm/vec3.hpp>

class MovementComponent final : public Component {
public:
	MovementComponent();
	~MovementComponent();

	glm::vec3 velocity;
	glm::vec3 relVel;
	glm::vec3 rotation;
	glm::vec3 constantAcceleration;
	glm::vec3 accelerationToAdd;

	glm::vec3 oldVelocity;
	glm::vec3 oldMovement;

	float airDrag;

	float updateableDt;

	static std::string ID;
};