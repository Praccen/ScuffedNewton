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

	private:
		Box* m_boundingBox;
	};

}