// internal
#include "ai_system.hpp"

/* FLOOR-BOUND ENEMY PHYSICS CONFIGURATION */
INTERNAL float enemyGravity = 500.f;
INTERNAL float enemyJumpSpeed = 200.f;
INTERNAL float enemyMaxMoveSpeed = 64.f;
INTERNAL float enemyMaxFallSpeed = 200.f;
INTERNAL float enemyGroundAcceleration = 700.f;
INTERNAL float enemyGroundDeceleration = 1000.f;
INTERNAL float enemyAirAcceleration = 650.f;
INTERNAL float enemyAirDeceleration = 500.f;
// LADDER
INTERNAL float ladderClimbSpeed = 64.f;

static float elapsedTime = 0.0f;

// a representation of pathable tiles
std::vector<std::vector<int>> levelTiles;


void AISystem::Step(float deltaTime)
{
	elapsedTime += deltaTime * 1000.0f;
	Entity playerEntity = registry.players.entities[0];
	TransformComponent& playerTransform = registry.transforms.get(playerEntity);
	if (elapsedTime >= 2000.0f) {
		for (Entity& enemy : registry.enemy.entities) {
			// MotionComponent& enemyMotion = registry.motions.get(enemy);
			// enemyMotion.velocity.x *= -1.f;
			EnemyAttack(enemy);
		}
		elapsedTime = 0.0f;
	}
	for (Entity& enemy : registry.enemy.entities) {
		// if entity in range of some amount of player (to reduce issues w/ run time) 
		TransformComponent& enemyTransform = registry.transforms.get(enemy);
		if (abs(playerTransform.position.x - enemyTransform.position.x) < 128 && abs(playerTransform.position.y - enemyTransform.position.y) < 128) {
			Physics(enemy, deltaTime);
			Pathfind(enemy);
		}
	}
}

void AISystem::EnemyAttack(Entity enemy_entity) {
	Enemy& enemy = registry.enemy.get(enemy_entity);
	MotionComponent& enemyMotion = registry.motions.get(enemy_entity);
	Entity playerEntity = registry.players.entities.front();
	MotionComponent& playerMotion = registry.motions.get(playerEntity);
	TransformComponent& player_transform = registry.transforms.get(playerEntity);
	TransformComponent& enemy_transform = registry.transforms.get(enemy_entity);
	vec2 diff_distance = player_transform.position - enemy_transform.position;
	if (diff_distance.x < 110 && diff_distance.x > -110 && diff_distance.y > 0 && diff_distance.y < 20) {
		float angle = atan2(diff_distance.y, diff_distance.x);
		vec2 velocity = vec2(cos(angle) * enemy.projectile_speed, sin(angle) * enemy.projectile_speed);
		createEnemyProjectile(enemy_transform.position, velocity, enemy_entity);
	}
}

// TODO this duplicates a lot of code from player physics, maybe want to abstract that 
void AISystem::Physics(Entity enemy_entity, float deltaTime) {
	bool bGrounded = false;
	bool bStillLaddered = false;
	bool bCollidedDirectlyAbove = false;
	MotionComponent& enemyMotion = registry.motions.get(enemy_entity);


	std::vector<CollisionEvent> relevantCollisions;
	std::vector<CollisionEvent> groundableCollisions;
	std::vector<CollisionEvent> directlyAboveCollisions;


	// Check if grounded or colliding above
	const auto& collisionsRegistry = registry.collisionEvents;
	for (u32 i = 0; i < collisionsRegistry.components.size(); ++i)
	{
		const CollisionEvent colEvent = collisionsRegistry.components[i];
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = colEvent.other;

		if (entity.GetTag() != enemy_entity.GetTag())
		{
			continue;
		}

		if (entity_other.GetTag() == TAG_PLAYERBLOCKABLE)
		{
			relevantCollisions.push_back(colEvent);

			CollisionComponent& enemyCollider = registry.colliders.get(entity);

			// Note(Kevin): this second collision check redundant right now but may become needed later - keep for now?
			CollisionInfo collisionCheck = CheckCollision(enemyCollider, registry.colliders.get(entity_other));
			if (collisionCheck.collides && abs(collisionCheck.collision_overlap.y) < abs(collisionCheck.collision_overlap.x))
			{
				if (collisionCheck.collision_overlap.y <= 0.f
					&& enemyMotion.velocity.y >= 0.f) // check we are not already moving up otherwise glitches.
				{
					groundableCollisions.push_back(colEvent);
					bGrounded = true;
				}
				if (collisionCheck.collision_overlap.y > 0.f)
				{
					directlyAboveCollisions.push_back(colEvent);
					bCollidedDirectlyAbove = true;
				}
			}
		}

	}

	// Check if actually grounded
	if (bGrounded)
	{
		for (auto gcol : groundableCollisions)
		{
			for (auto rcol : relevantCollisions)
			{
				if (gcol.collision_overlap.x == rcol.collision_overlap.x
					&& gcol.collision_overlap.y != rcol.collision_overlap.y)
				{
					/* Check we aren't in a wall */
					bGrounded = false;
					break;
				}
			}
		}
	}
	// Check if actually hit head
	if (bCollidedDirectlyAbove)
	{
		for (auto acol : directlyAboveCollisions)
		{
			for (auto rcol : relevantCollisions)
			{
				if (acol.collision_overlap.x == rcol.collision_overlap.x
					&& acol.collision_overlap.y != rcol.collision_overlap.y)
				{
					/* Check we aren't in a wall */
					bCollidedDirectlyAbove = false;
					break;
				}
			}
		}
	}

	if (bCollidedDirectlyAbove)
	{
		// Check velocity is negative otherwise we can reset velocity to 0 when already falling
		if (enemyMotion.velocity.y < 0.f) { enemyMotion.velocity.y = 0.f; }
	}

	if (bGrounded) {
		enemyMotion.velocity.y = 0;
	}
}

// TODO
	// GOALS:
	// make work when there's no path to player
	// make work when there's multiple spots the enemy would be ok with going
		// probably do this by finding list of desirable spots then going to closest one
	// make this work with close-to ideal goal spots not just ideal
	// make this run NOT every frame, maybe every 15 (?)
void AISystem::Pathfind(Entity enemy_entity) {
	bool isPlayerInAwarenessBubble = PlayerInAwarenessBubble(enemy_entity);
	if (!isPlayerInAwarenessBubble) {
		PatrolBehavior(enemy_entity);
	}
	else {
		PathBehavior(enemy_entity);
	}
}

void AISystem::PatrolBehavior(Entity enemy_entity) {
	PatrollingBehavior& patrollingBehavior = registry.patrollingBehaviors.get(enemy_entity);
	MotionComponent& motionComponent = registry.motions.get(enemy_entity);
	// should use acceleration (?)
	// should switch directions occasionally, need to add w.e. to keep track of where it should be
	// using random chance to switch for now to simulate it, I imagine this is very bad + its janky so remove it
	/*
	if (!patrollingBehavior.standStill) {
		if (abs(motionComponent.velocity.x) - abs(patrollingBehavior.patrolSpeed) != 0) {
			motionComponent.velocity.x = patrollingBehavior.patrolSpeed;
		}
		else if (rand() % (int)patrollingBehavior.patrolDistance <= 1) {
			motionComponent.velocity.x = -motionComponent.velocity.x;
		}
	}
	else {
		motionComponent.velocity.x = 0;
	}
	*/
	motionComponent.velocity.x = 0;
}

// NOTE: rn. all enemy speeds are same as player, so they run pretty fast towards the player. maybe should lower that
// it might be fine once the enemies actually respect collision
void AISystem::PathBehavior(Entity enemy_entity) {
	PathingBehavior& enemyPathingBehavior = registry.pathingBehaviors.get(enemy_entity);
	Entity player_entity = registry.players.entities.front();
	TransformComponent& playerTransformComponent = registry.transforms.get(player_entity);
	TransformComponent& enemyTransformComponent = registry.transforms.get(enemy_entity);
	MotionComponent& enemyMotionComponent = registry.motions.get(enemy_entity);
	if (enemyPathingBehavior.flyingType) {
		// (goal pos can be adjusted later TODO)
		//setup your location on grid, goal location on grid
		struct ListEntry {
			vec2 position;
			ListEntry* parent;
			int cost;
		};
		vec2 goalPos  = { ((int) (playerTransformComponent.position.x / 16)), ((int) (playerTransformComponent.position.y / 16)) };
		vec2 enemyPos = { ((int) (enemyTransformComponent.position.x / 16)),  ((int) (enemyTransformComponent.position.y / 16)) };

		//find path to goal if not on goal
		//go first step of that path towards goal tile
		// ASSUMING 0,0 tile is 0,0 on grid

		// form {position, cost, parent(position)}
		std::vector<ListEntry> openList;
		std::vector<ListEntry> closedList;
		ListEntry startPos = { enemyPos, NULL, 0 };
		openList.push_back(startPos);

		// TODO update this to use binary search/insert for speed
		while (!openList.empty()) 
		{
			int lowestIndexSoFar = 0;
			ListEntry currentPos = { enemyPos, NULL, 999 };
			for (int k = 0; k < openList.size(); k++) {
				if (openList[k].cost + Heuristic(openList[k].position, goalPos) < currentPos.cost + Heuristic(currentPos.position, goalPos)) {
					currentPos = openList[k];
					lowestIndexSoFar = k;
				}
			}
			openList.erase(openList.begin() + lowestIndexSoFar);
			closedList.push_back(currentPos);
			if (currentPos.position == goalPos) {
				// goal found
				vec2 prevPos;
				ListEntry currentEntry = currentPos;
				while (currentEntry.parent) {
					prevPos = currentEntry.position;
					currentEntry = *currentEntry.parent;
				}
				prevPos -= enemyPos;
				enemyMotionComponent.velocity = prevPos * 10.f;
			}
			std::vector<vec2> adjacentSquares;
			vec2 above = { currentPos.position[0],     currentPos.position[1] - 1 };
			vec2 right = { currentPos.position[0] + 1, currentPos.position[1] };
			vec2 below = { currentPos.position[0],     currentPos.position[1] + 1 };
			vec2 left  = { currentPos.position[0] - 1, currentPos.position[1] };
			printf("%f   %f, %f   %f, %f    %f, %f    %f, %f   %f   \n", above[0], above[1], below[0], below[1], right[0], right[1], left[0], left[1], goalPos[0], goalPos[1]);
			adjacentSquares.push_back(above);
			adjacentSquares.push_back(right);
			adjacentSquares.push_back(below);
			adjacentSquares.push_back(left );
			for (vec2 adjacentSquare : adjacentSquares) {
				if (adjacentSquare[0] >= 0 && adjacentSquare[1] >= 0 && adjacentSquare[0] < levelTiles.size() && adjacentSquare[1] < levelTiles[0].size()) {
					if (levelTiles[adjacentSquare[0]][adjacentSquare[1]]) {
						bool inClosedList = false;
						for (ListEntry& const closedEntry : closedList) {
							vec2 position = closedEntry.position;
							if (position[0] == adjacentSquare[0] && position[1] == adjacentSquare[1]) {
								inClosedList = true;
								break;
							}
						}
						if (inClosedList) {
							continue;
						}
						bool inOpenList = false;
						ListEntry* openListVersion;
						for (ListEntry listEntry : openList) {
							if (listEntry.position[0] == adjacentSquare[0] && listEntry.position[1] == adjacentSquare[1]) {
								inClosedList = true;
								openListVersion = &listEntry;
								break;
							}
						}
						if (inOpenList) {
							if (currentPos.cost + 1 < openListVersion->cost) {
								openListVersion->parent = &currentPos;
							}
						}
						else {
							ListEntry addEntry = { adjacentSquare, &currentPos, currentPos.cost + 1 };
							openList.push_back(addEntry);
						}
					}
				}
			}
		}

	}
	else {
		if (playerTransformComponent.position.x > enemyTransformComponent.position.x) {
			enemyMotionComponent.acceleration.x = enemyPathingBehavior.pathSpeed;
		}
		else {
			enemyMotionComponent.acceleration.x = -enemyPathingBehavior.pathSpeed;
		}
	}

}

bool AISystem::PlayerInAwarenessBubble(Entity enemy_entity) {
	Entity player_entity = registry.players.entities.front();
	TransformComponent& playerTransformComponent = registry.transforms.get(player_entity);
	TransformComponent& enemyTransformComponent = registry.transforms.get(enemy_entity);
	VisionComponent& visionComponent = registry.visionComponents.get(enemy_entity);
	vec2 relativePosition = abs(playerTransformComponent.position - enemyTransformComponent.position);
	float sightRadius = visionComponent.sightRadius;
	if (relativePosition.y < sightRadius && relativePosition.x < sightRadius) {
		return true;
	}
	return false;
}

void AISystem::Init(std::vector<std::vector<int>> newLevelTiles) {
	levelTiles = newLevelTiles;
}

// make DE-INIT

// Manhattan distance heuristic for heuristic search
float AISystem::Heuristic(vec2 startPos, vec2 goalPos) {
	return abs(startPos[0] - goalPos[0]) + abs(startPos[1] - goalPos[1]);
}

