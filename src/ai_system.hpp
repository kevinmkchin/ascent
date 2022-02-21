#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_init.hpp"

class AISystem
{
public:
	void Step(float deltaTime);
	void EnemyAttack(Entity enemy_entity);
};

