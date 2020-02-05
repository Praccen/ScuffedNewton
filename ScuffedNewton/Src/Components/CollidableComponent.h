#pragma once
#include "Component.h"

// Name is confusing. This is simply a flag where only entities with this component are inserted into the octree
class CollidableComponent : public Component {
public:
	CollidableComponent(bool simpleCollisionAllowed = false);
	~CollidableComponent();

	bool allowSimpleCollision; //Flag to let things collide with the entity's bounding box instead of mesh

	static std::string ID;
};