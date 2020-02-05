#pragma once

#include <iostream>

#include "DataStructures/Scene.h"

#ifdef SCUFFEDNEWTON_EXPORTS
#define SCUFFEDNEWTON_API __declspec(dllexport)
#else
#define SCUFFEDNEWTON_API __declspec(dllimport)
#endif

Scene scene;

// ----DLL functions----
extern "C" SCUFFEDNEWTON_API void update(float dt);

extern "C" SCUFFEDNEWTON_API int getNewObjectId();

extern "C" SCUFFEDNEWTON_API void loadMesh(int entityId, void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize);

extern "C" SCUFFEDNEWTON_API void setModelMatrixPointer(int entityId, glm::mat4 * modelMatrix);

extern "C" SCUFFEDNEWTON_API void addComponentToEntity(int entityId, int compType);
// ---------------------