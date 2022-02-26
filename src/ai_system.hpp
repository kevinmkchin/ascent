#pragma once

#include <vector>
#include <random>
#include <cmath>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_init.hpp"

class AISystem
{
public:
	void Step(float deltaTime);
	void EnemyAttack(Entity enemy_entity);
	void Pathfind(Entity enemy_entity);
	bool PlayerInAwarenessBubble(Entity enemy_entity);
	void PathBehavior(Entity enemy_entity);
	void PatrolBehavior(Entity enemy_entity);
	//float Heuristic(vec2 pos1, vec2 pos2);
	//void InitializeAISystem();
	//void AccessLevelSchematic(float pos);
};

