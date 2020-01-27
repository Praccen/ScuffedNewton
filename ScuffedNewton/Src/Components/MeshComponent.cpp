#include "../pch.h"
#include "MeshComponent.h"

#include "../DataTypes/Mesh.h"

MeshComponent::MeshComponent() {
	ID = "MeshComponent";
	mesh = SN_NEW Mesh();
}

MeshComponent::~MeshComponent() {
	delete mesh;
}

