#include "../pch.h"
#include "BoundingBoxComponent.h"

#include "../DataTypes/BoundingBox.h"

std::string BoundingBoxComponent::ID = "BoundingBoxComponent";

BoundingBoxComponent::BoundingBoxComponent() { 
	m_boundingBox = SN_NEW BoundingBox();
}

BoundingBoxComponent::~BoundingBoxComponent() {
	delete m_boundingBox;
}

BoundingBox* BoundingBoxComponent::getBoundingBox() const {
	return m_boundingBox;
}
