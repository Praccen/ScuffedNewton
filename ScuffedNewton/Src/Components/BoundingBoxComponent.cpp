#include "../pch.h"
#include "BoundingBoxComponent.h"

#include "../DataTypes/BoundingBox.h"

BoundingBoxComponent::BoundingBoxComponent() {
	boundingBox = SN_NEW BoundingBox();
}

BoundingBoxComponent::~BoundingBoxComponent()
{
}
