#pragma once

#include "Component.h"

namespace Scuffed {

	class Mesh;

	class MeshComponent : public Component {
	public:
		MeshComponent();
		~MeshComponent();

		Mesh* mesh = nullptr;

		static std::string ID;
	};

}