#pragma once

#include <map>
#include <string>

class BoundingBox;
class Component;
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

	template<typename ComponentType, typename... Targs>
	ComponentType* addComponent(Targs... args);

	template<typename ComponentType>
	void removeComponent();

	template<typename ComponentType>
	ComponentType* getComponent();

	template<typename ComponentType>
	bool hasComponent() const;

private:
	unsigned int m_id;

	BoundingBox* m_boundingBox;
	Mesh* m_mesh;
	bool m_hasModel;
	bool m_allowSimpleCollision;
	bool m_collidable;

	std::map<std::string, Component*> m_components;
};