#include "../pch.h"
#include "CollisionComponent.h"

namespace Scuffed {

	std::string CollisionComponent::ID = "CollisionComponent";

	CollisionComponent::CollisionComponent(bool simpleCollisions) {
		drag = 25.0f;
		bounciness = 0.0f;
		padding = -1.0f;
		onGround = false;
		doSimpleCollisions = simpleCollisions;
	}

	CollisionComponent::~CollisionComponent() {

	}

}