#include "../pch.h"
#include "UpdateBoundingBoxSystem.h"

#include "../Components/Components.h"

#include "../DataTypes/Entity.h"

#include "../DataTypes/Mesh.h"

#include "../Shapes/Box.h"

namespace Scuffed {

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

			auto transformationMatrix = transform->getMatrixWithUpdate();

			//Recalculate min and max
			for (int j = 0; j < mesh->mesh->getNumberOfVertices(); j++) {
				checkDistances(minPositions, maxPositions, mesh->mesh->getVertexPosition(j));
			}

			boundingBox->getBoundingBox()->setHalfSize((maxPositions - minPositions) * 0.5f);
			boundingBox->getBoundingBox()->setOrigin(minPositions + (maxPositions - minPositions) * 0.5f);
			boundingBox->getBoundingBox()->setBaseMatrix(transformationMatrix);
		}
		else {
			glm::mat4 transformationMatrix = transform->getMatrixWithUpdate();
			boundingBox->getBoundingBox()->setBaseMatrix(transformationMatrix);
			//boundingBox->getBoundingBox()->setPosition(transform->getTranslation() + glm::vec3(0.0f, boundingBox->getBoundingBox()->getHalfSize().y, 0.0f));
		}
	}

	void UpdateBoundingBoxSystem::recalculateBoundingBoxPosition(Entity* e) {
		BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();
		glm::mat4 transformationMatrix = transform->getMatrixWithUpdate();
		boundingBox->getBoundingBox()->setBaseMatrix(transformationMatrix);
	}

	bool UpdateBoundingBoxSystem::addEntity(Entity* entity) {
		if (BaseSystem::addEntity(entity)) {
			recalculateBoundingBoxFully(entity);
			return true;
		}
		return false;
	}

	void UpdateBoundingBoxSystem::update(float dt) {
		//std::cout << "UpdateBoundingBoxSystem system ran\n";

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
			}
		}
	}
}