#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

struct CollisionInfo
{
    vec2 collision_overlap = {0.f, 0.f};
    bool collides = false;
};

CollisionInfo CheckCollision(TransformComponent& transform1, CollisionComponent& collider1,
                             TransformComponent& transform2, CollisionComponent& collider2);

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float deltaTime);

	PhysicsSystem()
	{
	}
};