#include "pch.h"

#include "ScuffedNewton.h"

#include <string>

#include "DataTypes/Entity.h"
#include "Components/Components.h"
#include "DataTypes/Mesh.h"

#ifdef SCUFFEDNEWTON_EXPORTS
#define SCUFFEDNEWTON_API __declspec(dllexport)
#else
#define SCUFFEDNEWTON_API __declspec(dllimport)
#endif

SCUFFEDNEWTON_API void update(float dt) {
	scene.update(dt);
}

SCUFFEDNEWTON_API int getNewObjectId() {
	return scene.addEntity();
}

SCUFFEDNEWTON_API void loadMesh(int entityId, void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize) {
	//Find entity
	Entity* e = scene.getEntity(entityId);
	if (e) {
		MeshComponent* comp = e->getComponent<MeshComponent>();
		// Check if entity has mesh component
		if (comp) {
			// Load data
			comp->mesh->loadData(data, size, vertexSize, positionOffset, positionSize);
		}
		else {
			// Create component and load data
			comp = e->addComponent<MeshComponent>();
			comp->mesh->loadData(data, size, vertexSize, positionOffset, positionSize);
		}
	}
}

SCUFFEDNEWTON_API void bindModelMatrix(int entityId, glm::mat4** matrix) {
	//Find entity
	Entity* e = scene.getEntity(entityId);
	if (e) {
		TransformComponent* comp = e->getComponent<TransformComponent>();
		// Check if entity has mesh component
		if (comp) {
			// Bind pointer
			comp->bindMatrixPointer(matrix);
		}
		else {
			// Create component and bind pointer
			comp = e->addComponent<TransformComponent>();
			comp->bindMatrixPointer(matrix);
		}
	}
}

SCUFFEDNEWTON_API void bindPosition(int entityId, glm::vec3** positionVector) {
	//Find entity
	Entity* e = scene.getEntity(entityId);
	if (e) {
		TransformComponent* comp = e->getComponent<TransformComponent>();
		// Check if entity has mesh component
		if (comp) {
			// Bind pointer
			comp->bindPositionPointer(positionVector);
		}
		else {
			// Create component and bind pointer
			comp = e->addComponent<TransformComponent>();
			comp->bindPositionPointer(positionVector);
		}
	}
}

SCUFFEDNEWTON_API void addComponentToEntity(int entityId, int compType) {
	Entity* e = scene.getEntity(entityId);
	if (e) {
		switch (compType) {
		case 0: e->addComponent<BoundingBoxComponent>();  break;
		case 1: e->addComponent<CollidableComponent>();  break;
		case 2: e->addComponent<CollisionComponent>();  break;
		case 3: e->addComponent<MeshComponent>();  break;
		case 4: e->addComponent<MovementComponent>();  break;
		case 5: e->addComponent<RagdollComponent>(); break;
		case 6: e->addComponent<SpeedLimitComponent>();  break;
		case 7: e->addComponent<TransformComponent>(); break;
		default: break;
		}
	}
}
