#pragma once

#include <iostream>

#include "DataStructures/Scene.h"
#include "Interface.h"

#ifdef SCUFFEDNEWTON_EXPORTS
#define SCUFFEDNEWTON_API __declspec(dllexport)
#else
#define SCUFFEDNEWTON_API __declspec(dllimport)
#endif

namespace Scuffed {

	// ----DLL functions----
	extern "C" SCUFFEDNEWTON_API Scuffed::Interface * newInterface();
	// ---------------------

}