#include "../pch.h"
#include "CollisionComponent.h"

CollisionComponent::CollisionComponent(bool simpleCollisions) {
	ID = "CollisionComponent";
	drag = 25.0f;
	bounciness = 0.0f;
	padding = -1.0f;
	onGround = false;
	doSimpleCollisions = simpleCollisions;
}

CollisionComponent::~CollisionComponent() {

}
