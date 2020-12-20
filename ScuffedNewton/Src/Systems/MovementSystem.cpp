#include "../pch.h"
#include "MovementSystem.h"

#include "../Components/Components.h"

#include "../DataTypes/Entity.h"

namespace Scuffed {

	MovementSystem::MovementSystem() {
		requiredComponents["TransformComponent"] = true;
		requiredComponents["PhysicalBodyComponent"] = true;
	}

	void MovementSystem::update(float dt) {
		//std::cout << "Movement system ran\n";

		for (auto& e : entities) {
			TransformComponent* transform = e->getComponent<TransformComponent>();
			PhysicalBodyComponent* movement = e->getComponent<PhysicalBodyComponent>();

			if (glm::length2(movement->velocity) < Utils::instance()->epsilon) {
				movement->velocity = { 0.f, 0.f, 0.f };
			}

			// Update velocity
			movement->velocity += (movement->constantAcceleration + movement->accelerationToAdd) * dt;

			// Reset additional acceleration
			movement->accelerationToAdd = glm::vec3(0.0f);
		}
	}

}