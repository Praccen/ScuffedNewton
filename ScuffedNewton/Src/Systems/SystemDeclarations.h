#pragma once

// Ordered by name
class CollisionSystem;
class MovementPostCollisionSystem;
class MovementSystem;
class OctreeAddRemoverSystem;
class SpeedLimitSystem;
class UpdateBoundingBoxSystem;

struct Systems {
	CollisionSystem*				collisionSystem				= nullptr;
	MovementPostCollisionSystem*	movementPostCollisionSystem = nullptr;
	MovementSystem*					movementSystem				= nullptr;
	OctreeAddRemoverSystem*			octreeAddRemoverSystem		= nullptr;
	SpeedLimitSystem*				speedLimitSystem			= nullptr;
	UpdateBoundingBoxSystem*		updateBoundingBoxSystem		= nullptr;
};