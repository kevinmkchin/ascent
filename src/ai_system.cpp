// internal
#include "ai_system.hpp"

/* FLOOR-BOUND ENEMY PHYSICS CONFIGURATION */
INTERNAL float enemyGravity = 400.f;
INTERNAL float enemyJumpSpeed = 200.f;
// INTERNAL float enemyMaxMoveSpeed = 64.f;
INTERNAL float enemyMaxFallSpeed = 200.f;
INTERNAL float enemyGroundAcceleration = 700.f;
INTERNAL float enemyGroundDeceleration = 1000.f;
INTERNAL float enemyAirAcceleration = 650.f;
INTERNAL float enemyAirDeceleration = 500.f;
// LADDER
INTERNAL float ladderClimbSpeed = 64.f;

static float elapsedTime = 0.0f;
float elapsedAICycleTime = 0.0f;

// a representation of pathable tiles
std::vector<std::vector<int>> levelTiles;


void AISystem::Step(float deltaTime)
{
	elapsedTime += deltaTime * 1000.0f;
	HandleSpriteSheetFrame(deltaTime);
	elapsedAICycleTime += deltaTime * 1000.0f;
	Entity playerEntity = registry.players.entities[0];
	TransformComponent& playerTransform = registry.transforms.get(playerEntity);
	if (elapsedTime >= 2000.0f) // TODO: Should probably handle each enemy separately for attack frequency
	{
		for (Entity& enemy : registry.enemy.entities)
		{
			if (!registry.deathTimers.has(enemy)) {
				EnemyAttack(enemy);
			}
		}
		elapsedTime = 0.0f;
	}

	for (int i = 0; i < registry.enemy.size(); ++i)
	{
		Enemy& enemyComponent = registry.enemy.components[i];
		const Entity& enemy = registry.enemy.entities[i];

		if (registry.deathTimers.has(enemy)) {
			DeathTimer& time = registry.deathTimers.get(enemy);
			time.elapsed_ms -= deltaTime * 1000.f;
			enemyComponent.playerHurtCooldown = time.elapsed_ms;
		}
		else {
			if (enemyComponent.playerHurtCooldown > 0.f)
			{
				enemyComponent.playerHurtCooldown -= deltaTime;
			}

			TransformComponent& enemyTransform = registry.transforms.get(enemy);
			if (abs(playerTransform.position.x - enemyTransform.position.x) < 500 && abs(playerTransform.position.y - enemyTransform.position.y) < 500) {
				Physics(enemy, deltaTime);
			}
		}
	}

	//	if (elapsedAICycleTime >= 20.0f) {
	for (int i = 0; i < registry.enemy.size(); ++i)
	{
		Enemy& enemyComponent = registry.enemy.components[i];
		const Entity& enemy = registry.enemy.entities[i];
		// if entity in range of some amount of player (to reduce issues w/ run time) 

		if (!registry.deathTimers.has(enemy)) {
			TransformComponent& enemyTransform = registry.transforms.get(enemy);
			if (abs(playerTransform.position.x - enemyTransform.position.x) < 500 && abs(playerTransform.position.y - enemyTransform.position.y) < 500) {
				Pathfind(enemy);
				MotionComponent& enemyMotionComponent = registry.motions.get(enemy);
				("Enemy motion x : %f, enemy motion y: %f \n", enemyMotionComponent.velocity.x, enemyMotionComponent.velocity.y);
			}
			elapsedAICycleTime = 0.f;
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
	// TODO This projectile attack should be made into a "ProjectileAttackBehaviour" Component.
	// then only enemies that have that behaviour component can perform this attack whereas right now
	// every single enemy can perform this projectile attack

	// temporary check to prevent flying enemies from using projectile attack. remove later.
	PathingBehavior& enemyPathingBehavior = registry.pathingBehaviors.get(enemy_entity);
	if(enemyPathingBehavior.flyingType)
	{
		return;
	}

	Enemy& enemy = registry.enemy.get(enemy_entity);
	MotionComponent& enemyMotion = registry.motions.get(enemy_entity);
	Entity playerEntity = registry.players.entities.front();
	MotionComponent& playerMotion = registry.motions.get(playerEntity);
	TransformComponent& player_transform = registry.transforms.get(playerEntity);
	TransformComponent& enemy_transform = registry.transforms.get(enemy_entity);
	vec2 diff_distance = player_transform.position - enemy_transform.position;
	if (diff_distance.x < 100 && diff_distance.x > -100 && diff_distance.y > 0 && diff_distance.y < 20){
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
	MotionComponent enemyMotion = registry.motions.get(enemy_entity);


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

		if (entity != enemy_entity)
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
		enemyMotion.velocity.y = 0.f;
	}
}

// TODO
	// GOALS:
	// make work when there's no path to player (CHECK FOR FLYING)
	// make work when there's multiple spots the enemy would be ok with going ( NOT CHECK )
		// probably do this by finding list of desirable spots then going to closest one
	// make this work with close-to ideal goal spots not just ideal (probably do this by doing the multiple spot searching above? maybe if max iter fails for goal, go to OK spot
	// make this run NOT every frame, maybe every 15 (this is done by elapsed ms rn)
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
void AISystem::PathBehavior(Entity enemy_entity) {
	PathingBehavior& enemyPathingBehavior = registry.pathingBehaviors.get(enemy_entity);
	Entity player_entity = registry.players.entities.front();
	TransformComponent& playerTransformComponent = registry.transforms.get(player_entity);
	TransformComponent& enemyTransformComponent = registry.transforms.get(enemy_entity);
	MotionComponent& enemyMotionComponent = registry.motions.get(enemy_entity);

	// TODO(Caleb): Need to set terminalVelocity and y acceleration like this in PatrolBehavior function too
		// TODO this is done every time.. ?
	enemyMotionComponent.terminalVelocity.x = enemyPathingBehavior.pathSpeed;
	enemyMotionComponent.terminalVelocity.y = enemyMaxFallSpeed;

	if (playerTransformComponent.position.x > enemyTransformComponent.position.x) 
	{
		enemyMotionComponent.acceleration.x = enemyGroundAcceleration;
	}
	else 
	{
		enemyMotionComponent.acceleration.x = -enemyGroundAcceleration;
	}
	if (enemyPathingBehavior.flyingType) {
		// (goal pos can be adjusted later TODO)
		struct ListEntry {
			vec2 position;
			int parent;
			int cost;
		};

		vec2 goalPos   = { (int) ((playerTransformComponent.position.x + 1) / 16), (int) ((playerTransformComponent.position.y + 1) / 16) };
		vec2 enemyPos  = { (int) ((enemyTransformComponent.position.x + 1)  / 16), (int) ((enemyTransformComponent.position.y + 1)  / 16) };

		//printf("%f %f enemypos\n", enemyPos.x, enemyPos.y);
		////printf("%f %f goalPos\n", goalPos.x, goalPos.y);

		if (goalPos[0] == enemyPos[0] && goalPos[1] == enemyPos[1]) {
			return;
		}

		std::vector<ListEntry> openList;
		std::vector<ListEntry> closedList;
		ListEntry startPos = { enemyPos, NULL, 0 };
		openList.push_back(startPos);

		// TODO update this to use binary search/insert for speed
		int maxIter = 100;
		int currIter = 0;
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
				ListEntry currentEntry = currentPos;
				vec2 prevPos = currentEntry.position;
				currentEntry.parent = currentPos.parent;
				while (currentEntry.position != entriesSoFar[currentEntry.parent].position) {
					prevPos = currentEntry.position;
					currentEntry = entriesSoFar[currentEntry.parent];
				}
				vec2 direction = prevPos - enemyPos;
				//printf("direction x: %f, direction y: %f\n", direction.x, direction.y);
				// Clipping (being unable to path properly because of tile hitting non center of enemy) resolving section
				enemyMotionComponent.velocity = direction * 25.f; // (enemyMotionComponent.velocity / vec2{ 8, 8 }) +
				auto& enemyCollider = registry.colliders.get(enemy_entity);
				float spriteWidth  = enemyCollider.collision_pos.x;
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
				if (isClippingBRD || isClippingTRU) {
					printf("clip\n");
					enemyMotionComponent.velocity.x = -20;
				}
				else if (isClippingBLD || isClippingTLU) {
					printf("clip\n");
					enemyMotionComponent.velocity.x = 20;
				}
				else if (isClippingBRR || isClippingBLL) {
					printf("clip\n");
					enemyMotionComponent.velocity.y = -20;
				}
				else if (isClippingTRR || isClippingTLL) {
					printf("clip\n");
					enemyMotionComponent.velocity.y = 20;
				}
				enemyMotionComponent.acceleration.x = 0;
				enemyMotionComponent.acceleration.y = 0;
				printf("ai:\n enemyMotionComponent x: %f, enemyMotionComponent y: %f, tag: %i\n done\n", enemyMotionComponent.velocity.x, enemyMotionComponent.velocity.y, enemy_entity.GetTagAndID());
				return;
			}
			std::vector<vec2> adjacentSquares;
			vec2 above = { currentPos.position[0],     currentPos.position[1] - 1 };
			vec2 right = { currentPos.position[0] + 1, currentPos.position[1] };
			vec2 below = { currentPos.position[0],     currentPos.position[1] + 1 };
			vec2 left  = { currentPos.position[0] - 1, currentPos.position[1] };
			adjacentSquares.push_back(above);
			adjacentSquares.push_back(right);
			adjacentSquares.push_back(below);
			adjacentSquares.push_back(left );
			for (vec2 adjacentSquare : adjacentSquares) {
				if (adjacentSquare[0] >= 0 && adjacentSquare[1] >= 0 && adjacentSquare[0] < levelTiles.size() && adjacentSquare[1] < levelTiles[0].size()) {
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
	else {
		enemyMotionComponent.acceleration.y = enemyGravity;
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

// make DE-INIT?

// Manhattan distance heuristic for heuristic search
float AISystem::Heuristic(vec2 startPos, vec2 goalPos) {
	return abs(startPos[0] - goalPos[0]) + abs(startPos[1] - goalPos[1]);
}

