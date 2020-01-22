#pragma once

class BoundingBox;
class Mesh;

class Entity {
public:
	Entity();
	virtual ~Entity();

	void setCollidable(bool status);
	void setHasModel(bool status);

	unsigned int getId() const;

	BoundingBox* getBoundingBox() const;

	Mesh* getMesh() const;

	bool hasModel() const;
	bool isCollidable() const;
	bool allowSimpleCollision() const;

private:
	unsigned int m_id;

	BoundingBox* m_boundingBox;
	Mesh* m_mesh;
	bool m_hasModel;
	bool m_allowSimpleCollision;
	bool m_collidable;
};