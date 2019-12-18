#pragma once

#include <iostream>

#ifdef SCUFFEDNEWTON_EXPORTS
#define SCUFFEDNEWTON_API __declspec(dllexport)
#else
#define SCUFFEDNEWTON_API __declspec(dllimport)
#endif

// ----DLL functions----
extern "C" SCUFFEDNEWTON_API void testVoid();

extern "C" SCUFFEDNEWTON_API const char* testString();

extern "C" SCUFFEDNEWTON_API void update(float dt);
// ---------------------