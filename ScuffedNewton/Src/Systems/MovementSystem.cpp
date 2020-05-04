#include "../pch.h"
#include "MovementSystem.h"

#include "../Components/Components.h"

#include "../DataTypes/Entity.h"

namespace Scuffed {

	MovementSystem::MovementSystem() {
		requiredComponents["TransformComponent"] = true;
		requiredComponents["MovementComponent"] = true;
	}

	void MovementSystem::update(float dt) {
		//std::cout << "Movement system ran\n";

		for (auto& e : entities) {
			TransformComponent* transform = e->getComponent<TransformComponent>();
			MovementComponent* movement = e->getComponent<MovementComponent>();

			// Update velocity
			movement->velocity += (movement->constantAcceleration + movement->accelerationToAdd) * dt;

			// Reset additional acceleration
			movement->accelerationToAdd = glm::vec3(0.0f);

			// Set initial value which might be changed in CollisionSystem
			movement->updateableDt = dt;
		}
	}

}