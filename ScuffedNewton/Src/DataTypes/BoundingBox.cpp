#include "../pch.h"

#include "BoundingBox.h"
#include "../Shapes/Box.h"

namespace Scuffed {

	BoundingBox::BoundingBox() {
		m_position = glm::vec3(0.0f);
		m_halfSize = glm::vec3(0.5f);
		m_hasChanged = false;
		m_cornersNeedUpdate = true;
		prepareCorners();

		glm::vec3 planes[6] = {
			{m_halfSize.x, 0.f, 0.f},
			{-m_halfSize.x, 0.f, 0.f},
			{0.f, m_halfSize.y, 0.f},
			{0.f, -m_halfSize.y, 0.f},
			{0.f, 0.f, m_halfSize.z},
			{0.f, 0.f, -m_halfSize.z}
		};

		m_box = SN_NEW Box(planes, m_position);
		m_boxNeedsUpdate = false;
	}

	BoundingBox::~BoundingBox() {
		delete m_box;
	}

	void BoundingBox::updateCorners() {
		m_corners[0] = m_position + glm::vec3(-m_halfSize.x, m_halfSize.y, -m_halfSize.z); //Left Top Close - 0
		m_corners[1] = m_position + glm::vec3(m_halfSize.x, m_halfSize.y, -m_halfSize.z); //Right Top Close - 1
		m_corners[2] = m_position + glm::vec3(-m_halfSize.x, -m_halfSize.y, -m_halfSize.z); //Left Bottom Close - 2
		m_corners[3] = m_position + glm::vec3(m_halfSize.x, -m_halfSize.y, -m_halfSize.z); //Right Bottom Close - 3
		m_corners[4] = m_position + glm::vec3(-m_halfSize.x, m_halfSize.y, m_halfSize.z); //Left Top Far - 4
		m_corners[5] = m_position + glm::vec3(m_halfSize.x, m_halfSize.y, m_halfSize.z); //Right Top Far - 5
		m_corners[6] = m_position + glm::vec3(-m_halfSize.x, -m_halfSize.y, m_halfSize.z); //Left Bottom Far - 6
		m_corners[7] = m_position + glm::vec3(m_halfSize.x, -m_halfSize.y, m_halfSize.z); //Right Bottom Far - 7

		m_cornersNeedUpdate = false;
	}

	const bool BoundingBox::getChange() {
		bool theChange = m_hasChanged;
		m_hasChanged = false;
		return theChange;
	}

	const glm::vec3& BoundingBox::getPosition() const {
		return m_position;
	}

	const glm::vec3& BoundingBox::getHalfSize() const {
		return m_halfSize;
	}

	void BoundingBox::prepareCorners() {
		if (m_cornersNeedUpdate) {
			updateCorners();
		}
	}

	const glm::vec3* BoundingBox::getCornersWithUpdate() {
		prepareCorners();
		return m_corners;
	}

	const glm::vec3* BoundingBox::getCornersWithoutUpdate() const {
		return m_corners;
	}

	void BoundingBox::setPosition(const glm::vec3& position) {
		m_position = position;
		m_hasChanged = true;
		m_cornersNeedUpdate = true;
		m_boxNeedsUpdate = true;
	}

	void BoundingBox::setHalfSize(const glm::vec3& size) {
		m_halfSize = size;
		m_hasChanged = true;
		m_cornersNeedUpdate = true;
		m_boxNeedsUpdate = true;
	}

	Box* BoundingBox::getBox() {
		if (m_boxNeedsUpdate) {
			glm::vec3 planes[6] = {
			{m_halfSize.x, 0.f, 0.f},
			{-m_halfSize.x, 0.f, 0.f},
			{0.f, m_halfSize.y, 0.f},
			{0.f, -m_halfSize.y, 0.f},
			{0.f, 0.f, m_halfSize.z},
			{0.f, 0.f, -m_halfSize.z}
			};

			m_box->setData(planes, m_position);

			m_boxNeedsUpdate = false;
		}

		return m_box;
	}

}