#include "pch.h"

#include "ScuffedNewton.h"

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
	scene.setCollidable(entityId, status);
}