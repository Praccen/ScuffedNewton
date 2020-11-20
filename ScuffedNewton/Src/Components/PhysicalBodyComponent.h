#pragma once

#include "Component.h"
#include "../DataStructures/Octree.h"
#include <glm/gtc/quaternion.hpp>


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

		std::vector<Entity*> restingContacts;

		static std::string ID;
	};

}