#pragma once

#include <glm/vec3.hpp>

namespace Scuffed {
	
	class Box;

	class BoundingBox {
	private:
		glm::vec3 m_position;
		glm::vec3 m_halfSize;

		glm::vec3 m_corners[8];

		bool m_hasChanged;
		bool m_cornersNeedUpdate;
		bool m_boxNeedsUpdate;

		void updateCorners();

		Box* m_box;

	private:
		friend class Octree;
		const bool getChange(); //Only access this from Octree::updateRec

	public:
		BoundingBox();
		virtual ~BoundingBox();

		virtual const glm::vec3& getPosition() const;
		virtual const glm::vec3& getHalfSize() const;

		virtual void prepareCorners();
		virtual const glm::vec3* getCornersWithUpdate();
		virtual const glm::vec3* getCornersWithoutUpdate() const;


		virtual void setPosition(const glm::vec3& position);
		virtual void setHalfSize(const glm::vec3& size);

		virtual Box* getBox();
	};

}