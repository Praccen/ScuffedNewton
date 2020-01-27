#pragma once

#include "Component.h"

class BoundingBox;

class BoundingBoxComponent : public Component {
	BoundingBoxComponent();
	~BoundingBoxComponent();

	BoundingBox* boundingBox = nullptr;
};