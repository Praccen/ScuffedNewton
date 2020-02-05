#include "pch.h"

#include "ScuffedNewton.h"

#include <string>

#include "DataTypes/Entity.h"
#include "Components/Components.h"

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
	//Entity* e = scene.getEntity(entityId);
	//if (e) {
	//	/*e->getMesh()->loadData(data, size, vertexSize, positionOffset, positionSize);
	//	e->setHasModel(true);*/
	//}
	assert(false);
}

SCUFFEDNEWTON_API void setModelMatrixPointer(int entityId, glm::mat4* modelMatrix) {
	/*Entity* e = scene.getEntity(entityId);
	if (e) {
		e->getMesh()->setModelMatrixPointer(modelMatrix);
	}*/
	assert(false);
}

SCUFFEDNEWTON_API void addComponentToEntity(int entityId, int compType) {
	Entity* e = scene.getEntity(entityId);
	if (e) {
		switch (compType) {
		case 0: e->addComponent<BoundingBoxComponent>();  break;
		case 1: e->addComponent<CollidableComponent>();  break;
		case 2: e->addComponent<MeshComponent>();  break;
		case 3: e->addComponent<MovementComponent>();  break;
		case 4: e->addComponent<RagdollComponent>(); break;
		case 5: e->addComponent<SpeedLimitComponent>();  break;
		default: break;
		}
	}
}
