#pragma once

#include "Component.h"

class BoundingBox;

class BoundingBoxComponent : public Component {
public:
	BoundingBoxComponent();
	~BoundingBoxComponent();

	BoundingBox* boundingBox = nullptr;

	static std::string ID;
};