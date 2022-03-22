// internal
#include "ai_system.hpp"

/* FLOOR-BOUND ENEMY PHYSICS CONFIGURATION */
INTERNAL float enemyGravity = 250.f;
INTERNAL float enemyJumpSpeed = 125.f;
INTERNAL float enemyMaxMoveSpeed = 64.f;
INTERNAL float enemyMaxFallSpeed = 125.f;
INTERNAL float enemyGroundAcceleration = 700.f;
INTERNAL float enemyGroundDeceleration = 1000.f;
INTERNAL float enemyAirAcceleration = 650.f;
INTERNAL float enemyAirDeceleration = 500.f;
// LADDER
INTERNAL float ladderClimbSpeed = 64.f;

float elapsedAICycleTime = 0.0f;

// a representation of pathable tiles
std::vector<std::vector<int>> levelTiles;


void AISystem::Step(float deltaTime)
{
	float elapsedTime = deltaTime * 1000.0f;
	HandleSpriteSheetFrame(deltaTime);
	elapsedAICycleTime += elapsedTime;
	Entity playerEntity = registry.players.entities[0];
	TransformComponent& playerTransform = registry.transforms.get(playerEntity);

	for (int i = 0; i < registry.enemy.size(); ++i)
	{
		Enemy& enemyComponent = registry.enemy.components[i];
		const Entity& enemy = registry.enemy.entities[i];

		// Attacks
		if (enemyComponent.elapsedTime >= enemyComponent.attackCooldown) {
			if (!registry.deathTimers.has(enemy)) {
				EnemyAttack(enemy);
				enemyComponent.elapsedTime = 0.f;
			}
		}
		else {
			TransformComponent& enemyTransform = registry.transforms.get(enemy);
			if (abs(playerTransform.position.x - enemyTransform.position.x) < 600 && abs(playerTransform.position.y - enemyTransform.position.y) < 600) {
				enemyComponent.elapsedTime += elapsedTime;
			}
		}

		// Dying / colliding with player
		if (registry.deathTimers.has(enemy)) {
			DeathTimer& time = registry.deathTimers.get(enemy);
			time.elapsed_ms -= deltaTime * 1000.f;
			enemyComponent.playerHurtCooldown = time.elapsed_ms;
			if (time.elapsed_ms <= 0.f) {
				
				int random_count = random(1, 3);

				vec2 expPosition = registry.transforms.get(enemy).position;
				vec2 expPosition1 = { expPosition.x + 10.f, expPosition.y };
				vec2 expPosition2 = { expPosition.x - 10.f, expPosition.y };

				registry.remove_all_components_of(enemy);

				int coin_or_exp = random(1, 3);

				if (coin_or_exp == 2) {
					for (int i = 1; i <= random_count; i++) {

						if (i == 1) {
							createCoins(expPosition);
						}

						if (i == 2) {
							createCoins(expPosition1);
						}

						else {
							createCoins(expPosition2);
						}

					}
				}
				else {

					for (int i = 1; i <= random_count; i++) {

						if (i == 1) {
							createExp(expPosition);
						}

						if (i == 2) {
							createExp(expPosition1);
						}

						else {
							createExp(expPosition2);
						}

					}

				}
			}
		}
		else {
			if (enemyComponent.playerHurtCooldown > 0.f)
			{
				enemyComponent.playerHurtCooldown -= deltaTime;
			}
			if (registry.walkingBehaviors.has(enemy)) {
				EnemyJumping(enemy, deltaTime);
			}
		}
	}

	// Pathing
	if (elapsedAICycleTime >= 30.0f) {
		for (int i = 0; i < registry.enemy.size(); ++i)
		{
			Enemy& enemyComponent = registry.enemy.components[i];
			const Entity& enemy = registry.enemy.entities[i];
			// if entity in range of some amount of player (to reduce issues w/ run time) 

			if (!registry.deathTimers.has(enemy)) {
				TransformComponent& enemyTransform = registry.transforms.get(enemy);
				if (abs(playerTransform.position.x - enemyTransform.position.x) < 500 && abs(playerTransform.position.y - enemyTransform.position.y) < 500) {
					Pathfind(enemy, elapsedTime);
					MotionComponent& enemyMotionComponent = registry.motions.get(enemy);
				}
				elapsedAICycleTime = 0.f;
			}
		}
	}

}

void AISystem::HandleSpriteSheetFrame(float deltaTime)
{
	for (int i = 0; i < registry.sprites.size(); i++) {
		SpriteComponent& sprite = registry.sprites.components[i];
		Entity& entity = registry.sprites.entities[i];

		if (registry.enemy.has(entity) && sprite.sprite_sheet) {
			MotionComponent& motion = registry.motions.get(entity);

			float x_velocity = motion.velocity.x;
			bool faceRight = motion.facingRight;
			bool reversed = sprite.reverse;
			int prev_state = sprite.selected_animation;

			if (registry.deathTimers.has(entity)) {
				// death
				sprite.selected_animation = 2;
			}
			else if (x_velocity != 0.f) {
				// run
				sprite.selected_animation = 1;
			}
			else {
				// idle
				sprite.selected_animation = 0;
			}

			bool aligned = sprite.faceRight ? (faceRight != reversed) : (faceRight == reversed);
			sprite.current_frame = (aligned && prev_state == sprite.selected_animation)
				? sprite.current_frame : 0;

			sprite.reverse = sprite.faceRight ? !faceRight : faceRight;
		}
	}
}

void AISystem::EnemyAttack(Entity enemy_entity) {
	if (registry.rangedBehaviors.has(enemy_entity))
	{
		Enemy& enemy = registry.enemy.get(enemy_entity);
		RangedBehavior enemyRangedBehavior = registry.rangedBehaviors.get(enemy_entity);
		MotionComponent& enemyMotion = registry.motions.get(enemy_entity);
		Entity playerEntity = registry.players.entities.front();
		MotionComponent& playerMotion = registry.motions.get(playerEntity);
		TransformComponent& player_transform = registry.transforms.get(playerEntity);
		TransformComponent& enemy_transform = registry.transforms.get(enemy_entity);
		vec2 diff_distance = player_transform.position - enemy_transform.position;
		if (diff_distance.x < 100 && diff_distance.x > -100 && diff_distance.y > -50 && diff_distance.y < 50) {
			if (enemyRangedBehavior.lobbing) {

				vec2 velocity = vec2(0.2f * enemy.projectile_speed, -2.0f * enemy.projectile_speed);
				vec2 acceleration = vec2(0.f, 250.f);

				float random_change = 15.f;
				float percent_diff = (float)rand() / RAND_MAX;
				int direction = rand() % 2;
				if (direction) {
					random_change *= -1.f;
				}
				velocity.x += random_change * percent_diff;

				vec2 neg_velocity = { -velocity.x, velocity.y };
				
				createEnemyLobbingProjectile(enemy_transform.position, velocity, acceleration, enemy_entity);
				createEnemyLobbingProjectile(enemy_transform.position, neg_velocity, acceleration, enemy_entity);
			}
			else {
				float angle = atan2(diff_distance.y, diff_distance.x);
				vec2 velocity = vec2(cos(angle) * enemy.projectile_speed, sin(angle) * enemy.projectile_speed);
				createEnemyProjectile(enemy_transform.position, velocity, enemy_entity);
			}
		}
	}
	// TODO implement actual melee attacks from enemies, not just contact damage (MeleeBehavior component) but thats not for me to do yet
}

// TODO
	// make work when there's multiple spots the enemy would be ok with going
		// probably do this by finding list of desirable spots then going to closest one
void AISystem::Pathfind(Entity enemy_entity, float elapsedTime) {
	bool isPlayerInAwarenessBubble = PlayerInAwarenessBubble(enemy_entity);
	if (!isPlayerInAwarenessBubble) {
		PatrolBehavior(enemy_entity, elapsedTime);
	}
	else {
		PathBehavior(enemy_entity);
	}
}

void AISystem::PatrolBehavior(Entity enemy_entity, float elapsedTime) {
	if (registry.patrollingBehaviors.has(enemy_entity)) {
		PatrollingBehavior& patrollingBehavior = registry.patrollingBehaviors.get(enemy_entity);
		MotionComponent& motionComponent = registry.motions.get(enemy_entity);

		motionComponent.terminalVelocity.x = patrollingBehavior.patrolSpeed;
		motionComponent.terminalVelocity.y = enemyMaxFallSpeed;

		if (patrollingBehavior.standStill) {
			motionComponent.velocity.x = 0;
		}
		else {
			if (motionComponent.velocity.x == 0) {
				motionComponent.velocity.x = patrollingBehavior.patrolSpeed;
			}
			patrollingBehavior.currentPatrolTime += elapsedTime;
			if (patrollingBehavior.maxPatrolTime <= patrollingBehavior.currentPatrolTime) {
				motionComponent.velocity.x = -patrollingBehavior.patrolSpeed;
				patrollingBehavior.currentPatrolTime = 0;
			}
		}
	}
}

void AISystem::PathBehavior(Entity enemy_entity) {
	// TODO abstract this for easier extension (probably focus on the expanding squares option!)
	if (registry.pathingBehaviors.has(enemy_entity)) {
		PathingBehavior& enemyPathingBehavior = registry.pathingBehaviors.get(enemy_entity);
		Entity player_entity = registry.players.entities.front();
		TransformComponent& playerTransformComponent = registry.transforms.get(player_entity);
		TransformComponent& enemyTransformComponent = registry.transforms.get(enemy_entity);
		MotionComponent& enemyMotionComponent = registry.motions.get(enemy_entity);

		enemyMotionComponent.terminalVelocity.x = enemyPathingBehavior.pathSpeed;
		enemyMotionComponent.terminalVelocity.y = enemyMaxFallSpeed;

		if (registry.flyingBehaviors.has(enemy_entity)) {
			struct ListEntry {
				vec2 position;
				int parent;
				int cost;
			};

			// GOAL POS IS NOT CORRECTLY CONSIDERED HERE! be aware.
			vec2 goalPos  = { (int)((playerTransformComponent.position.x + 1) / 16), (int)((playerTransformComponent.position.y + 1) / 16) };
			vec2 enemyPos = { (int)((enemyTransformComponent.position.x + 1)  / 16), (int)((enemyTransformComponent.position.y + 1)  / 16) };

			if (goalPos[0] == enemyPos[0] && goalPos[1] == enemyPos[1]) {
				return;
			}

			std::vector<ListEntry> openList;
			std::vector<ListEntry> closedList;
			ListEntry startPos = { enemyPos, NULL, 0 };
			openList.push_back(startPos);

			// TODO update this to use binary search/insert for speed if its a problem speed wise
			uint8 maxIter = 50;
			uint8 currIter = 0;
			std::vector<ListEntry> entriesSoFar;
			while (!openList.empty())
			{
				if (maxIter < currIter) {
					return;
				}
				int lowestIndexSoFar = 0;
				ListEntry currentPos = { enemyPos, NULL, 999 };
				for (int k = 0; k < openList.size(); k++) {
					if (openList[k].cost + Heuristic(openList[k].position, goalPos) < currentPos.cost + Heuristic(currentPos.position, goalPos)) {
						currentPos = openList[k];
						lowestIndexSoFar = k;
					}
				}
				entriesSoFar.push_back(currentPos);
				openList.erase(openList.begin() + lowestIndexSoFar);
				closedList.push_back(currentPos);
				if (currentPos.position == goalPos) {
					// goal found
					// go to the start of the path, and set direction as next step to take
					ListEntry currentEntry = currentPos;
					vec2 prevPos = currentEntry.position;
					currentEntry.parent = currentPos.parent;
					while (currentEntry.position != entriesSoFar[currentEntry.parent].position) {
						prevPos = currentEntry.position;
						currentEntry = entriesSoFar[currentEntry.parent];
					}
					/* TODO make the open air A* feel better
					desire for A* :
						if we want to go from Y to A and Z is on the path, and if the tiles to the right and below
						Y are open, go diagonal. still need to check for clipping. this should go after setting default direction, for elegance sake
						XXXXX
						XYXXX
						XXZXA
						XXXXX
					possible solution: draw a line from Y to A, if its not obstructed then perform the optimal.
					alternate solution: use acceleration. I forsee this breaking tight searching
					if (currIter >= 2) {
						vec2 secondLastEntryPosition = entriesSoFar[currIter - 1].position;
						vec2 thirdLastEntryPosition  = entriesSoFar[currIter - 2].position;
						// NW on path
						// SW on path
						// SE on path
						// NE on path
					}
					*/

					vec2 direction = prevPos - enemyPos;

					// Clipping (being unable to path properly because of tile hitting non center of enemy) resolving section
					enemyMotionComponent.velocity = direction * 25.f; 
					auto& enemyCollider = registry.colliders.get(enemy_entity);
					float spriteWidth = enemyCollider.collision_pos.x;
					float spriteHeight = enemyCollider.collision_pos.y;
					vec2 enemyPosR = { ((int)((enemyTransformComponent.position.x + spriteWidth) / 16)),  ((int)(enemyTransformComponent.position.y / 16)) };
					vec2 enemyPosL = { ((int)((enemyTransformComponent.position.x - spriteWidth) / 16)),  ((int)(enemyTransformComponent.position.y / 16)) };
					vec2 enemyPosB = { ((int)(enemyTransformComponent.position.x / 16)),  ((int)((enemyTransformComponent.position.y + spriteHeight) / 16)) };
					vec2 enemyPosA = { ((int)(enemyTransformComponent.position.x / 16)),  ((int)((enemyTransformComponent.position.y - spriteHeight) / 16)) };
					bool isClippingTLL = direction.x < 0 && enemyPosA != enemyPos && levelTiles[(size_t)enemyPos.x - 1][(size_t)enemyPos.y - 1] != 0;
					bool isClippingTLU = direction.y < 0 && enemyPosL != enemyPos && levelTiles[(size_t)enemyPos.x - 1][(size_t)enemyPos.y - 1] != 0;
					bool isClippingTRR = direction.x > 0 && enemyPosA != enemyPos && levelTiles[(size_t)enemyPos.x + 1][(size_t)enemyPos.y - 1] != 0;
					bool isClippingTRU = direction.y < 0 && enemyPosR != enemyPos && levelTiles[(size_t)enemyPos.x + 1][(size_t)enemyPos.y - 1] != 0;
					bool isClippingBLL = direction.x < 0 && enemyPosB != enemyPos && levelTiles[(size_t)enemyPos.x - 1][(size_t)enemyPos.y + 1] != 0;
					bool isClippingBLD = direction.y > 0 && enemyPosL != enemyPos && levelTiles[(size_t)enemyPos.x - 1][(size_t)enemyPos.y + 1] != 0;
					bool isClippingBRR = direction.x > 0 && enemyPosB != enemyPos && levelTiles[(size_t)enemyPos.x + 1][(size_t)enemyPos.y + 1] != 0;
					bool isClippingBRD = direction.y > 0 && enemyPosR != enemyPos && levelTiles[(size_t)enemyPos.x + 1][(size_t)enemyPos.y + 1] != 0;
					int clipVelocity = 15;
					if (isClippingBRD || isClippingTRU) {
						enemyMotionComponent.velocity.x = -clipVelocity;
					}
					else if (isClippingBLD || isClippingTLU) {
						enemyMotionComponent.velocity.x = clipVelocity;
					}
					else if (isClippingBRR || isClippingBLL) {
						enemyMotionComponent.velocity.y = -clipVelocity;
					}
					else if (isClippingTRR || isClippingTLL) {
						enemyMotionComponent.velocity.y = clipVelocity;
					}
					enemyMotionComponent.acceleration.x = 0;
					enemyMotionComponent.acceleration.y = 0;
					return;
				}

				std::vector<vec2> adjacentSquares;
				vec2 above = { currentPos.position[0],     currentPos.position[1] - 1 };
				vec2 right = { currentPos.position[0] + 1, currentPos.position[1] };
				vec2 below = { currentPos.position[0],     currentPos.position[1] + 1 };
				vec2 left = { currentPos.position[0] - 1, currentPos.position[1] };

				adjacentSquares.push_back(above);
				adjacentSquares.push_back(right);
				adjacentSquares.push_back(below);
				adjacentSquares.push_back(left);

				for (vec2 adjacentSquare : adjacentSquares) {
					if (adjacentSquare[0] >= 0 && adjacentSquare[1] >= 0 && adjacentSquare[0] < levelTiles.size() && adjacentSquare[1] < levelTiles[0].size()) {
						// if its a free tile
						if (levelTiles[(size_t)adjacentSquare[0]][(size_t)adjacentSquare[1]] == 0) {
							bool inClosedList = false;
							for (const ListEntry& closedEntry : closedList) {
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
									openListVersion->parent = currIter;
								}
							}
							else {
								ListEntry addEntry = { adjacentSquare, currIter, currentPos.cost + 1 };
								openList.push_back(addEntry);
							}
						}
					}
				}
				currIter++;
			}
		}
		else if (registry.walkingBehaviors.has(enemy_entity)) {
		// if dumb, else
		auto& walkingBehavior = registry.walkingBehaviors.get(enemy_entity);
			if (walkingBehavior.stupid) {
				enemyMotionComponent.acceleration.y = enemyGravity;
				if (playerTransformComponent.position.x > enemyTransformComponent.position.x) {
					// decelerate faster than accelerate, use the global level var?
					if (enemyMotionComponent.velocity.x > 0) {
						enemyMotionComponent.acceleration.x = enemyPathingBehavior.pathSpeed;
					}
					else {
						enemyMotionComponent.acceleration.x = enemyPathingBehavior.pathSpeed * 1.3;
					}
				}
				else {
					if (enemyMotionComponent.velocity.x < 0) {
						enemyMotionComponent.acceleration.x = -enemyPathingBehavior.pathSpeed;
					}
					else {
						enemyMotionComponent.acceleration.x = -enemyPathingBehavior.pathSpeed * 1.3;
					}
				}
			}
			else {
				enemyMotionComponent.acceleration.y = enemyGravity;
				// smart walker
				struct ListEntry {
					vec2 position;
					int parent;
					int cost;
				};

				vec2 goalPos  = { (int)((playerTransformComponent.position.x + 1) / 16), (int)((playerTransformComponent.position.y + 1) / 16) };
				vec2 enemyPos = { (int)((enemyTransformComponent.position.x  + 1) / 16), (int)((enemyTransformComponent.position.y  + 1) / 16) };

				if (goalPos[0] == enemyPos[0] && goalPos[1] == enemyPos[1]) {
					return;
				}

				std::vector<ListEntry> openList;
				std::vector<ListEntry> closedList;
				ListEntry startPos = { enemyPos, NULL, 0 };
				openList.push_back(startPos);

				// TODO update this to use binary search/insert for speed if its a problem speed wise
				uint8 maxIter = 25;
				uint8 currIter = 0;
				std::vector<ListEntry> entriesSoFar;
				while (!openList.empty())
				{
					if (maxIter < currIter) {
						return;
					}
					int lowestIndexSoFar = 0;
					ListEntry currentPos = { enemyPos, NULL, 999 };
					for (int k = 0; k < openList.size(); k++) {
						if (openList[k].cost + Heuristic(openList[k].position, goalPos) < currentPos.cost + Heuristic(currentPos.position, goalPos)) {
							currentPos = openList[k];
							lowestIndexSoFar = k;
						}
					}
					entriesSoFar.push_back(currentPos);
					openList.erase(openList.begin() + lowestIndexSoFar);
					closedList.push_back(currentPos);
					if (currentPos.position == goalPos) {
						// goal found
						// go to the start of the path, and set direction as next step to take
						ListEntry currentEntry = currentPos;
						vec2 prevPos = currentEntry.position;
						currentEntry.parent = currentPos.parent;
						while (currentEntry.position != entriesSoFar[currentEntry.parent].position) {
							prevPos = currentEntry.position;
							currentEntry = entriesSoFar[currentEntry.parent];
						}

						vec2 direction = prevPos - enemyPos;
						// decelerate faster than accelerate, use the global level var?
						walkingBehavior.jumpRequest = false;
						if (direction.x != 0) {
							enemyMotionComponent.acceleration.x = direction.x * enemyPathingBehavior.pathSpeed;
							if (direction.x > 0 && enemyMotionComponent.velocity.x < 0 || direction.x < 0 && enemyMotionComponent.velocity.x > 0) {
								enemyMotionComponent.acceleration.x *= 1.75;
							}
						}
						else {
							if (direction.y < 0) {
								walkingBehavior.jumpRequest = true;
							}
						}
						return;
					}

					std::vector<vec2> adjacentSquares;
					vec2 above = { currentPos.position[0],     currentPos.position[1] - 1 };
					vec2 right = { currentPos.position[0] + 1, currentPos.position[1] };
					vec2 below = { currentPos.position[0],     currentPos.position[1] + 1 };
					vec2 left  = { currentPos.position[0] - 1, currentPos.position[1] };

					if (levelTiles[below.x][below.y] == 0) { // if current tile has no floor
						adjacentSquares.push_back(below);
					}
					else { // current tile has a floor (PROBABLY DOESNT WORK WITH LADDERS HERE..)
						adjacentSquares.push_back(above);        // if current has floor
					}
					adjacentSquares.push_back(right);
					adjacentSquares.push_back(left);

					for (vec2 adjacentSquare : adjacentSquares) {
						if (adjacentSquare[0] >= 0 && adjacentSquare[1] >= 0 && adjacentSquare[0] < levelTiles.size() && adjacentSquare[1] < levelTiles[0].size()) {
							// if its a free tile
							if (levelTiles[(size_t)adjacentSquare[0]][(size_t)adjacentSquare[1]] == 0) {
								bool inClosedList = false;
								for (const ListEntry& closedEntry : closedList) {
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
										openListVersion->parent = currIter;
									}
								}
								else {
									ListEntry addEntry = { adjacentSquare, currIter, currentPos.cost + 1 };
									openList.push_back(addEntry);
								}
							}
						}
					}
					currIter++;
				}

			}
		}
		else {
			// you can't do any sort of movement but you want to path - you're not set properly.
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
	if (visionComponent.hasAggro) {
		if (relativePosition.y < sightRadius * 1.25 && relativePosition.x < sightRadius * 1.5) {
			return true;
		}
		visionComponent.hasAggro = false;
	}
	else {
		if (relativePosition.y < sightRadius && relativePosition.x < sightRadius) {
			return true;
		}
		visionComponent.hasAggro = true;
	}

	return false;
}

void AISystem::Init(std::vector<std::vector<int>> newLevelTiles) {
	levelTiles = newLevelTiles;
}

// Manhattan distance heuristic for heuristic search
float AISystem::Heuristic(vec2 startPos, vec2 goalPos) {
	return abs(startPos[0] - goalPos[0]) + abs(startPos[1] - goalPos[1]);
}

void AISystem::EnemyJumping(Entity enemy_entity, float deltaTime) {
	registry.motions.get(enemy_entity).acceleration.y = enemyGravity;
	auto& walkingBehavior = registry.walkingBehaviors.get(enemy_entity);
	if (walkingBehavior.stupid) {
		return;
	}
	MotionComponent& enemyMotion = registry.motions.get(enemy_entity);
	bool bGrounded = false;
	bool bStillLaddered = false;
	bool bCollidedDirectlyAbove = false;
	bool bJumpingAndAscending = enemyMotion.velocity.y < 0.f;

	std::vector<CollisionEvent> enemyRelevantCollisions;
	std::vector<CollisionEvent> groundableCollisions;
	std::vector<CollisionEvent> directlyAboveCollisions;

	// Check if grounded or colliding above
	const auto& collisionsRegistry = registry.collisionEvents;
	for (u32 i = 0; i < collisionsRegistry.components.size(); ++i)
	{
		const CollisionEvent colEvent = collisionsRegistry.components[i];
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = colEvent.other;

		if (entity.GetTagAndID() != enemy_entity.GetTagAndID())
		{
			continue;
		}

		if (entity_other.GetTag() == TAG_PLAYERBLOCKABLE)
		{
			enemyRelevantCollisions.push_back(colEvent);

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

		/** If pressing up, colliding with a ladder, and not jumping and ascending, then we can climb ladder */
		/* IMPLEMENT LADDER CLIMBING..?
		if (bUpKeyPressed && entity_other.GetTag() == TAG_LADDER && !bJumpingAndAscending)
		{
			bLaddered = true;
		}
		if (bLaddered && entity_other.GetTag() == TAG_LADDER)
		{
			bStillLaddered = true;
		}

	}

	if (!bStillLaddered)
	{
		bLaddered = false;
	}
	*/
	}

	// Check if actually grounded
	if (bGrounded)
	{
		for (auto gcol : groundableCollisions)
		{
			for (auto pcol : enemyRelevantCollisions)
			{
				if (gcol.collision_overlap.x == pcol.collision_overlap.x
					&& gcol.collision_overlap.y != pcol.collision_overlap.y)
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
			for (auto pcol : enemyRelevantCollisions)
			{
				if (acol.collision_overlap.x == pcol.collision_overlap.x
					&& acol.collision_overlap.y != pcol.collision_overlap.y)
				{
					/* Check we aren't in a wall */
					bCollidedDirectlyAbove = false;
					break;
				}
			}
		}
	}
	/*
	if (bLaddered)
	{
		playerMotion.acceleration.y = 0.0;
		playerMotion.velocity.y = 0.f;
		if (bUpKeyPressed)
		{
			playerMotion.velocity.y += -ladderClimbSpeed;
		}
		if (bDownKeyPressed)
		{
			playerMotion.velocity.y += ladderClimbSpeed;
		}
		bGrounded = true;
	}
	*/

	if (bGrounded)
	{
		/*
		if (!bLaddered)
		{
			// Only set velocity to 0 if we are not climbing ladder
			playerMotion.velocity.y = 0.f;
		}
		coyoteTimer = coyoteTimeDefaultSeconds;
		*/
		enemyMotion.velocity.y = 0.f;
	}

	if (bCollidedDirectlyAbove)
	{
		// Check velocity is negative otherwise we can reset velocity to 0 when already falling
		if (enemyMotion.velocity.y < 0.f) { enemyMotion.velocity.y = 0.f; }
	}

	if (walkingBehavior.jumpRequest)
	{
		// Actually jump
		if (!bCollidedDirectlyAbove && (bGrounded))
		{
			walkingBehavior.jumpRequest = false;
			enemyMotion.velocity.y = -enemyJumpSpeed;
		}
	}
}
