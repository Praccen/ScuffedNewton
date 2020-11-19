#include "../pch.h"
#include "PhysicalBodyComponent.h"

namespace Scuffed {
	std::string PhysicalBodyComponent::ID = "PhysicalBodyComponent";

	PhysicalBodyComponent::PhysicalBodyComponent() {
		velocity = glm::vec3(0.f);
		rotation = glm::quat({0.f, 0.f, 0.f});
		constantAcceleration = glm::vec3(0.f);
		accelerationToAdd = glm::vec3(0.f);
			   
		frictionCoefficient = 0.2f;
		collisionCoefficient = 0.5f;

		weight = 1.0f;
	}

	PhysicalBodyComponent::~PhysicalBodyComponent() {

	}
}
