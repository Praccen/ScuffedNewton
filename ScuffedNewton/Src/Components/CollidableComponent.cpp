#include "../pch.h"
#include "CollidableComponent.h"

CollidableComponent::CollidableComponent(bool simpleCollisionAllowed) {
	ID = "CollidableComponent";
	allowSimpleCollision = simpleCollisionAllowed;
}

CollidableComponent::~CollidableComponent() {

}
