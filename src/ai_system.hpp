#pragma once

#include <vector>
#include <random>
#include <cmath>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_init.hpp"
#include "physics_system.hpp"

class AISystem
{
public:
	void Step(float deltaTime);
	void HandleSpriteSheetFrame(float deltaTime);
	void EnemyAttack(Entity enemy_entity);
	void Pathfind(Entity enemy_entity, float elapsedTime);
	void EnemyJumping(Entity enemy_entity, float deltaTime);
	bool PlayerInAwarenessBubble(Entity enemy_entity);
	void PathBehavior(Entity enemy_entity);
	void PatrolBehavior(Entity enemy_entity, float elapsedTime);
	void Init(std::vector<std::vector<int>> levelTiles);
	float Heuristic(vec2 pos1, vec2 pos2);
};

