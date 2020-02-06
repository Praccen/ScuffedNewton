#pragma once

#include "Component.h"

class BoundingBox;

class BoundingBoxComponent : public Component {
public:
	BoundingBoxComponent();
	~BoundingBoxComponent();

	BoundingBox* getBoundingBox() const;

	static std::string ID;

private:
	BoundingBox* m_boundingBox = nullptr;
};