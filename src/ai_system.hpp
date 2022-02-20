#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_init.hpp"

class AISystem
{
public:
	void step(float deltaTime);
	void Enemy_attack(Entity enemy_entity);
};

