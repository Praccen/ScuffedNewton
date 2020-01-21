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
extern "C" SCUFFEDNEWTON_API void testVoid();

extern "C" SCUFFEDNEWTON_API const char* testString();

extern "C" SCUFFEDNEWTON_API void update(float dt);

extern "C" SCUFFEDNEWTON_API int getNewObjectId();

extern "C" SCUFFEDNEWTON_API void setCollidable(int entityId, bool status);
// ---------------------