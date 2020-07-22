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

	void UpdateBoundingBoxSystem::recalculateBoundingBox(Entity* e) {
		MeshComponent* mesh = e->getComponent<MeshComponent>();
		BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();
		if (mesh && mesh->getChange()) {
			glm::vec3 minPositions(9999999.0f), maxPositions(-9999999.0f);

			int nrVertices = mesh->mesh->getNumberOfVertices();

			if (nrVertices > 0) {
				//Recalculate min and max
				for (int j = 0; j < nrVertices; j++) {
					checkDistances(minPositions, maxPositions, mesh->mesh->getVertexPosition(j));
				}
			}

			boundingBox->getBoundingBox()->setHalfSize((maxPositions - minPositions) * 0.5f);
			boundingBox->getBoundingBox()->setOrigin(minPositions + (maxPositions - minPositions) * 0.5f);

			auto transformationMatrix = transform->getMatrixWithUpdate();
			boundingBox->getBoundingBox()->setBaseMatrix(transformationMatrix);
		}
		else {
			glm::mat4 transformationMatrix = transform->getMatrixWithUpdate();
			boundingBox->getBoundingBox()->setBaseMatrix(transformationMatrix);
		}
	}

	bool UpdateBoundingBoxSystem::addEntity(Entity* entity) {
		if (BaseSystem::addEntity(entity)) {
			recalculateBoundingBox(entity);
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
				if (change > 0) {
					recalculateBoundingBox(e);
				}
			}
		}
	}
}