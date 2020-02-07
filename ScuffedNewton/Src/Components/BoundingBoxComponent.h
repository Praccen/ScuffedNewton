#pragma once

#include "Component.h"

class BoundingBox;

class BoundingBoxComponent : public Component {
public:
	BoundingBoxComponent();
	~BoundingBoxComponent();
	static std::string ID;

	BoundingBox* getBoundingBox() const;

	bool isStatic;

private:
	BoundingBox* m_boundingBox;
};