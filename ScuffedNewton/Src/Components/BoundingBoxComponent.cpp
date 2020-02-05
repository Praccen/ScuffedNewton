#include "../pch.h"
#include "BoundingBoxComponent.h"

#include "../DataTypes/BoundingBox.h"

std::string BoundingBoxComponent::ID = "BoundingBoxComponent";

BoundingBoxComponent::BoundingBoxComponent() {
	boundingBox = SN_NEW BoundingBox();
}

BoundingBoxComponent::~BoundingBoxComponent() {
	delete boundingBox;
}
