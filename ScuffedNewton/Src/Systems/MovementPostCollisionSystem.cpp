#include "../pch.h"

#include <algorithm>

#include "MovementPostCollisionSystem.h"
#include "../Components/Components.h"

#include "../DataTypes/Entity.h"
#include "../Calculations/Intersection.h"


namespace Scuffed {

	MovementPostCollisionSystem::MovementPostCollisionSystem() {
		requiredComponents["TransformComponent"] = true;
		requiredComponents["MovementComponent"] = true;
	}

	void MovementPostCollisionSystem::update(float dt) {
		//std::cout << "MovementPostCollision system ran\n";

		for (auto& e : entities) {
			TransformComponent* transform = e->getComponent<TransformComponent>();
			MovementComponent* movement = e->getComponent<MovementComponent>();

			momentum(e, dt);

			// Rotation
			if (movement->rotation != glm::vec3(0.0f)) {
				transform->rotate(movement->rotation * dt);
			}

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
			//glm::vec3 translation = movement->velocity * movement->updateableDt;
			if (translation != glm::vec3(0.0f)) {
				transform->translate(translation);
			}
			movement->oldMovement = translation;

			movement->oldVelocity = movement->velocity;
		}
	}

	void MovementPostCollisionSystem::momentum(Entity* e, float dt) {
		CollisionComponent* collision = e->getComponent<CollisionComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();
		MovementComponent* movement = e->getComponent<MovementComponent>();

		glm::vec3 manifoldOffset(0.f);

		//std::cout << collision->manifolds.size() << "\n";

		if (collision->manifolds.size() == 1) {
			manifoldOffset = collision->manifolds[0] - transform->getCenter() + movement->constantAcceleration * Intersection::dot(collision->manifolds[0] - transform->getCenter(), glm::normalize(movement->constantAcceleration));
		}
		else if (collision->manifolds.size() == 2) {
			float tempDot = Intersection::dot(glm::normalize(collision->manifolds[1] - collision->manifolds[0]), transform->getCenter());
			if (tempDot < 0) {
				manifoldOffset = collision->manifolds[0];
			}
			else if (tempDot > glm::length(collision->manifolds[1] - collision->manifolds[0])) {
				manifoldOffset = collision->manifolds[1];
			}
			else {
				manifoldOffset = collision->manifolds[0] + tempDot * glm::normalize(collision->manifolds[1] - collision->manifolds[0]);
			}

		}
		else if (collision->manifolds.size() > 2) {
			// Create triangles
			for (size_t i = 0; i < collision->manifolds.size() - 2; i++) {
				for (size_t j = i; j < collision->manifolds.size() - 1; j++) {
					for (size_t k = j; k < collision->manifolds.size(); k++) {
						if (Intersection::RayWithTriangle(transform->getCenter(), movement->constantAcceleration, collision->manifolds[i], collision->manifolds[j], collision->manifolds[k])) {
							// Center of mass is above collision manifold area
 							return;
						}
					}
				}
			}

			// Center of mass is not above collision manifold area, find manifoldOffset
			std::cout << "Manifold offset not found (lacks implementation)\n";
		}

		//movement->rotation += glm::cross(manifoldOffset, glm::vec3(0.0f, 1.0f, 0.0f));
	}

}