#pragma once

#include "Component.h"
#include <glm/gtc/quaternion.hpp>
#include "../Calculations/Intersection.h"

namespace Scuffed {
	class PhysicalBodyComponent : public Component {
	public:
		PhysicalBodyComponent();
		~PhysicalBodyComponent();

		glm::vec3 velocity;
		glm::quat rotation;
		glm::vec3 constantAcceleration;
		glm::vec3 accelerationToAdd;

		float frictionCoefficient;
		float collisionCoefficient;

		float mass;
		bool isConstraint;

		std::vector<Intersection::CollisionTimeInfo> restingContacts;

		static std::string ID;
	};

}