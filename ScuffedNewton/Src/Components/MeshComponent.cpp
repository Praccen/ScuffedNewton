#include "../pch.h"
#include "MeshComponent.h"

#include "../DataTypes/Mesh.h"

std::string MeshComponent::ID = "MeshComponent";

MeshComponent::MeshComponent() {
	mesh = SN_NEW Mesh();
}

MeshComponent::~MeshComponent() {
	delete mesh;
}

