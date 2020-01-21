#pragma once

class BoundingBox;

class Entity {
public:
	Entity();
	virtual ~Entity();

	void setCollidable(bool status);

	unsigned int getId() const;

	BoundingBox* getBoundingBox() const;

	bool hasModel() const;
	bool isCollidable() const;
	bool allowSimpleCollision() const;

private:
	unsigned int m_id;

	BoundingBox* m_boundingBox;
	bool m_hasModel;
	bool m_allowSimpleCollision;
	bool m_collidable;
};