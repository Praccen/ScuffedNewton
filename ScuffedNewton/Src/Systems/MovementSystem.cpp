#include "../pch.h"
#include "MovementSystem.h"

#include "../Components/Components.h"

#include "../DataTypes/Entity.h"


MovementSystem::MovementSystem() {
	//requiredComponents["TransformComponent"] = true;
	requiredComponents["MovementComponent"] = true;
	requiredComponents["RagdollComponent"] = true;
}

void MovementSystem::update(float dt) {
	for (auto& e : entities) {
		//TransformComponent* transform = e->getComponent<TransformComponent>();
		MovementComponent* movement = e->getComponent<MovementComponent>();

		// Update velocity
		movement->velocity += (movement->constantAcceleration + movement->accelerationToAdd) * dt;

		// Reset additional acceleration
		movement->accelerationToAdd = glm::vec3(0.0f);
		
		// Rotation
		if (movement->rotation != glm::vec3(0.0f)) {
			assert(false);
			//transform->rotate(movement->rotation * dt);
		}

		// Set initial value which might be changed in CollisionSystem
		movement->updateableDt = dt;
	}
}
