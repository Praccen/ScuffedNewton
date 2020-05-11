#include "../pch.h"
#include "MeshComponent.h"

#include "../DataTypes/Mesh.h"

namespace Scuffed {

	std::string MeshComponent::ID = "MeshComponent";

	MeshComponent::MeshComponent() {
		mesh = SN_NEW Mesh();
		m_hasChanged = true;
	}

	MeshComponent::~MeshComponent() {
		delete mesh;
	}

	void MeshComponent::notifyChange() {
		m_hasChanged = true;
	}

	const bool MeshComponent::getChange() {
		bool toReturn = m_hasChanged;
		m_hasChanged = false;
		return toReturn;
	}

}