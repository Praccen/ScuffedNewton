#include "../pch.h"
#include "CollidableComponent.h"

std::string CollidableComponent::ID = "CollidableComponent";

CollidableComponent::CollidableComponent(bool simpleCollisionAllowed) {
	allowSimpleCollision = simpleCollisionAllowed;
}

CollidableComponent::~CollidableComponent() {

}
