#pragma once

#include "Component.h"

namespace Scuffed {

	class Mesh;

	class MeshComponent : public Component {
	public:
		MeshComponent();
		~MeshComponent();

		virtual void notifyChange();

		Mesh* mesh = nullptr;
		bool useMeshCollision = true;

		static std::string ID;
		
	private:
		bool m_hasChanged;
		friend class UpdateBoundingBoxSystem;
		const bool getChange(); //Only access this from UpdateBoundingBoxSystem::update()
	};

}