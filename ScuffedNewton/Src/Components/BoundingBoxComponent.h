#pragma once

#include "Component.h"
#include "../DataTypes/BoundingBox.h"

namespace Scuffed {

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