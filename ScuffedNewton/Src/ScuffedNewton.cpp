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

namespace Scuffed {

	SCUFFEDNEWTON_API Interface* newInterface() {
		return SN_NEW Interface(); //Client needs to delete this
	}

}