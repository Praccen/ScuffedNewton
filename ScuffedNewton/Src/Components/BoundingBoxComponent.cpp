#include "../pch.h"
#include "BoundingBoxComponent.h"

#include "../Shapes/Box.h"
namespace Scuffed {
	std::string BoundingBoxComponent::ID = "BoundingBoxComponent";

	BoundingBoxComponent::BoundingBoxComponent() {
		m_boundingBox = SN_NEW Box(glm::vec3(0.5f), glm::vec3(0.f));
	}

	BoundingBoxComponent::~BoundingBoxComponent() {
		delete m_boundingBox;
	}

	Box* BoundingBoxComponent::getBoundingBox() const {
		return m_boundingBox;
	}

	
}
