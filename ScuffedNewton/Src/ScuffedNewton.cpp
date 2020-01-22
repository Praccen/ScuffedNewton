#include "pch.h"

#include "ScuffedNewton.h"

#include <string>

SCUFFEDNEWTON_API void testVoid() {
	std::cout << "Test void ran!\n";
}

SCUFFEDNEWTON_API const char* testString() {
	return "Hello world!";
}

SCUFFEDNEWTON_API void update(float dt) {
	scene.update(dt);
}

SCUFFEDNEWTON_API int getNewObjectId() {
	return scene.addEntity();
}

SCUFFEDNEWTON_API void setCollidable(int entityId, bool status) {
	Entity* e = scene.getEntity(entityId);
	if (e) {
		e->setCollidable(status);
	}
}

SCUFFEDNEWTON_API void loadMesh(int entityId, void** data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize) {
	Entity* e = scene.getEntity(entityId);
	if (e) {
		e->getMesh()->loadData(data, size, vertexSize, positionOffset, positionSize);
	}
}

SCUFFEDNEWTON_API void setModelMatrixPointer(int entityId, glm::mat4* modelMatrix) {
	Entity* e = scene.getEntity(entityId);
	if (e) {
		e->getMesh()->setModelMatrixPointer(modelMatrix);
	}
}

SCUFFEDNEWTON_API const char* testMeshPosition(int entityId) {
	std::string text = "Not found";

	Entity* e = scene.getEntity(entityId);
	if (e) {
		glm::vec3* tempPos = &e->getMesh()->getVertexPosition(0);

		text = std::to_string(tempPos->x) + ", " + std::to_string(tempPos->y) + ", " + std::to_string(tempPos->z);
	}

	return text.c_str();
}
