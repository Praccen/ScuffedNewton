#include "../pch.h"

#include "RagdollComponent.h"

std::string RagdollComponent::ID = "RagdollComponent";

RagdollComponent::RagdollComponent() {
	localCenterOfMass = {0.f, 0.f, 0.f};
}

RagdollComponent::~RagdollComponent() {

}

void RagdollComponent::addContactPoint(glm::vec3 localOffset, glm::vec3 halfSize) {
	contactPoints.emplace_back();
	contactPoints.back().boundingBox.setHalfSize(halfSize);
	contactPoints.back().localOffSet = localOffset;
}
