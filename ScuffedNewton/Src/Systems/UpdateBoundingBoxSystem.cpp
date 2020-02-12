#include "../pch.h"
#include "UpdateBoundingBoxSystem.h"

#include "../Components/Components.h"

#include "../DataTypes/Entity.h"

#include "../DataTypes/Mesh.h"

UpdateBoundingBoxSystem::UpdateBoundingBoxSystem() : BaseSystem() {
	requiredComponents["BoundingBoxComponent"] = true;
	requiredComponents["TransformComponent"] = true;
}

UpdateBoundingBoxSystem::~UpdateBoundingBoxSystem() {
}

void UpdateBoundingBoxSystem::checkDistances(glm::vec3& minVec, glm::vec3& maxVec, const glm::vec3& testVec) {
	if (testVec.x > maxVec.x) {
		maxVec.x = testVec.x;
	}
	if (testVec.x < minVec.x) {
		minVec.x = testVec.x;
	}
	if (testVec.y > maxVec.y) {
		maxVec.y = testVec.y;
	}
	if (testVec.y < minVec.y) {
		minVec.y = testVec.y;
	}
	if (testVec.z > maxVec.z) {
		maxVec.z = testVec.z;
	}
	if (testVec.z < minVec.z) {
		minVec.z = testVec.z;
	}
}

void UpdateBoundingBoxSystem::recalculateBoundingBoxFully(Entity* e) {
	MeshComponent* mesh = e->getComponent<MeshComponent>();
	BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	if (mesh) {
		glm::vec3 minPositions(9999999.0f), maxPositions(-9999999.0f);

		//Recalculate min and max
		for (int j = 0; j < mesh->mesh->getNumberOfVertices(); j++) {
			glm::vec3 posAfterTransform = glm::vec3(transform->getMatrixWithUpdate() * glm::vec4(mesh->mesh->getVertexPosition(j), 1.0f));
			checkDistances(minPositions, maxPositions, posAfterTransform);
		}

		boundingBox->getBoundingBox()->setHalfSize((maxPositions - minPositions) * 0.5f);
		boundingBox->getBoundingBox()->setPosition(minPositions + boundingBox->getBoundingBox()->getHalfSize());
	}
	else {
		boundingBox->getBoundingBox()->setPosition(transform->getTranslation() + glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	}

	transform->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	transform->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);
	/*boundingBox->getTransform()->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	boundingBox->getTransform()->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);*/
}

void UpdateBoundingBoxSystem::recalculateBoundingBoxPosition(Entity* e) {
	BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
	TransformComponent* transform = e->getComponent<TransformComponent>();
	glm::mat4 transformationMatrix = transform->getMatrixWithUpdate();
	boundingBox->getBoundingBox()->setPosition(glm::vec3(transformationMatrix[3]) + glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	transform->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	transform->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);
	/*boundingBox->getTransform()->setTranslation(boundingBox->getBoundingBox()->getPosition() - glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
	boundingBox->getTransform()->setScale(boundingBox->getBoundingBox()->getHalfSize() * 2.0f);*/
}

void UpdateBoundingBoxSystem::updateRagdollBoundingBoxes(Entity* e) {
	RagdollComponent* ragdollComp = e->getComponent<RagdollComponent>();
	TransformComponent* transComp = e->getComponent<TransformComponent>();

	for (size_t i = 0; i < ragdollComp->contactPoints.size(); i++) {
		ragdollComp->contactPoints[i].boundingBox.setPosition(glm::vec3(transComp->getMatrixWithUpdate() * glm::vec4(ragdollComp->contactPoints[i].localOffSet, 1.0f)));
		ragdollComp->contactPoints[i].transform.setTranslation(ragdollComp->contactPoints[i].boundingBox.getPosition() - glm::vec3(0.0f, ragdollComp->contactPoints[i].boundingBox.getHalfSize().y, 0.0f));
		ragdollComp->contactPoints[i].transform.setScale(ragdollComp->contactPoints[i].boundingBox.getHalfSize() * 2.0f);
	}
}

bool UpdateBoundingBoxSystem::addEntity(Entity* entity) {
	if (BaseSystem::addEntity(entity)) {
		recalculateBoundingBoxFully(entity);
		return true;
	}
	return false;
}

void UpdateBoundingBoxSystem::update(float dt) {
	for (auto& e : entities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		if (transform) {
			int change = transform->getChange();
			if (change > 1 && !e->getComponent<BoundingBoxComponent>()->isStatic) {
				recalculateBoundingBoxFully(e);
			}
			else if (change > 0) {
				recalculateBoundingBoxPosition(e);
			}

			if (e->hasComponent<RagdollComponent>()) {
				recalculateBoundingBoxFully(e);
				updateRagdollBoundingBoxes(e);
			}
		}
	}

	//std::cout << "UpdateBoundingBoxSystem system ran\n";

	// prepare matrixes and bounding boxes
	for (auto e : entities) {
		e->getComponent<BoundingBoxComponent>()->getBoundingBox()->prepareCorners();
		//std::cout << e->getId() << ", ";
	}
	//std::cout << "\n";
}
