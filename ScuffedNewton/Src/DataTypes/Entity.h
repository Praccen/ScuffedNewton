#pragma once

#include <map>
#include <string>

class BoundingBox;
class Component;
class Mesh;
class Scene;

class Entity {
public:
	Entity(Scene* scene);
	virtual ~Entity();

	unsigned int getId() const;

	template<typename ComponentType, typename... Targs>
	ComponentType* addComponent(Targs... args);

	template<typename ComponentType>
	void removeComponent();

	template<typename ComponentType>
	ComponentType* getComponent();

	template<typename ComponentType>
	bool hasComponent() const;

private:
	void addToSystems();
	void removeFromSystems();

private:
	unsigned int m_id;

	Scene* m_scene;

	BoundingBox* m_boundingBox;
	Mesh* m_mesh;
	bool m_hasModel;
	bool m_allowSimpleCollision;
	bool m_collidable;

	std::map<std::string, Component*> m_components;
};