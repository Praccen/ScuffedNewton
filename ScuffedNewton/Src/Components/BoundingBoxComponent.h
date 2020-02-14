#pragma once

#include "Component.h"

namespace Scuffed {

	class BoundingBox;

	class BoundingBoxComponent : public Component {
	public:
		BoundingBoxComponent();
		~BoundingBoxComponent();
		static std::string ID;

		virtual BoundingBox* getBoundingBox() const;

		bool isStatic;

	private:
		BoundingBox* m_boundingBox;
	};

}