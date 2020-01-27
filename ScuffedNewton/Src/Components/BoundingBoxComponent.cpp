#include "../pch.h"
#include "BoundingBoxComponent.h"

#include "../DataTypes/BoundingBox.h"

BoundingBoxComponent::BoundingBoxComponent() {
	ID = "BoundingBoxComponent";
	boundingBox = SN_NEW BoundingBox();
}

BoundingBoxComponent::~BoundingBoxComponent() {
	delete boundingBox;
}
