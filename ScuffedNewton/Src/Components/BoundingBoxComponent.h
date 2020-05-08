#pragma once

#include "Component.h"

namespace Scuffed {

	class Box;

	class BoundingBoxComponent : public Component {
	public:
		BoundingBoxComponent();
		~BoundingBoxComponent();
		static std::string ID;

		virtual Box* getBoundingBox() const;

		bool isStatic;



	private:
		Box* m_boundingBox;
		bool m_hasChanged;

	private:
		friend class Octree;
		const bool getChange(); //Only access this from Octree::updateRec
	};

}