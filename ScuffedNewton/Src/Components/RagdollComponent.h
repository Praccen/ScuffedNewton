#pragma once

#include "Component.h"

#include "../DataTypes/BoundingBox.h"
#include "../DataTypes/Transform.h"

namespace Scuffed {

	class RagdollComponent : public Component {
	public:
		RagdollComponent();
		~RagdollComponent();

		void addContactPoint(glm::vec3 localOffset, glm::vec3 halfSize);

	public:
		struct ContactPoint {
			BoundingBox boundingBox;
			glm::vec3 localOffSet;
			//std::vector<Octree::CollisionInfo> collisions;
			Transform transform;
		};

		std::vector<ContactPoint> contactPoints;

		glm::vec3 localCenterOfMass;

		static std::string ID;
	};

}