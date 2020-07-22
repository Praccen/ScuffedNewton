#pragma once
#include "Component.h"
#include "../DataStructures/Octree.h"

namespace Scuffed {

	class CollisionComponent : public Component {
	public:
		CollisionComponent(bool simpleCollisions = false);
		~CollisionComponent();

		float drag;
		float bounciness;
		bool onGround;
		bool doSimpleCollisions;

		std::vector<Octree::CollisionInfo> collisions; //Contains the info for current collisions
		std::vector<glm::vec3> manifolds;

		static std::string ID;
	};

}