#pragma once

#include "Component.h"

class Mesh;

class MeshComponent : public Component {
public:
	MeshComponent();
	~MeshComponent();

	Mesh* mesh = nullptr;
};