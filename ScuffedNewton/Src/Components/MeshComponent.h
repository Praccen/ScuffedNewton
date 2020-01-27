#pragma once

#include "Component.h"

class Mesh;

class MeshComponent : public Component {
	MeshComponent();
	~MeshComponent();

	Mesh* mesh = nullptr;
};