#include "../pch.h"

#include "MovementPostCollisionSystem.h"
#include "../Components/Components.h"

#include "../DataTypes/Entity.h"

MovementPostCollisionSystem::MovementPostCollisionSystem() {
	requiredComponents["TransformComponent"] = true;
	requiredComponents["MovementComponent"] = true;
}

void MovementPostCollisionSystem::update(float dt) {
	//std::cout << "MovementPostCollision system ran\n";

	// prepare matrixes and bounding boxes
	for (auto e : entities) {
		e->getComponent<BoundingBoxComponent>()->getBoundingBox()->prepareCorners();
		//std::cout << e->getId() << ", ";
	}
	//std::cout << "\n";

	for (auto& e : entities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		MovementComponent* movement = e->getComponent<MovementComponent>();

		// Apply air drag
		float saveY = movement->velocity.y;
		movement->velocity.y = 0;
		float vel = glm::length(movement->velocity);

		if (vel > 0.0f) {
			vel = glm::max(vel - movement->airDrag * dt, 0.0f);
			movement->velocity = glm::normalize(movement->velocity) * vel;
		}
		movement->velocity.y = saveY;

		// Update position with velocities after CollisionSystem has potentially altered them
		glm::vec3 translation = (movement->oldVelocity + movement->velocity) * (0.5f * movement->updateableDt);
		if (translation != glm::vec3(0.0f)) {
			transform->translate(translation);
		}
		movement->oldMovement = translation;

		movement->oldVelocity = movement->velocity;
	}
}
