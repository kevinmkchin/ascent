// internal
#include "ai_system.hpp"

static float elapsedTime = 0.0f;
// BUILD A STATIC REPRESENTATION OF TILES OF LEVEL?
// int* levelSchematic;

void AISystem::Step(float deltaTime)
{
	elapsedTime += deltaTime * 1000.0f;
	if (elapsedTime >= 2000.0f) {
		for (Entity& enemy : registry.enemy.entities) {
			// MotionComponent& enemyMotion = registry.motions.get(enemy);
			// enemyMotion.velocity.x *= -1.f;
			EnemyAttack(enemy);
		}
		elapsedTime = 0.0f;
	}
	for (Entity& enemy : registry.enemy.entities) {
		Pathfind(enemy);
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
	if (diff_distance.x < 110 && diff_distance.x > -110 && diff_distance.y > 0 && diff_distance.y < 20){
		float angle = atan2(diff_distance.y, diff_distance.x);
		vec2 velocity = vec2(cos(angle) * enemy.projectile_speed, sin(angle) * enemy.projectile_speed);
		createEnemyProjectile(enemy_transform.position, velocity, enemy_entity);
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
	} else {
		PathBehavior(enemy_entity);
	}
}

void AISystem::PatrolBehavior(Entity enemy_entity) {
	PatrollingBehavior& patrollingBehavior = registry.patrollingBehaviors.get(enemy_entity);
	MotionComponent& motionComponent = registry.motions.get(enemy_entity);
	// should use acceleration (?)
	// should switch directions occasionally, need to add w.e. to keep track of where it should be
	// using random chance to switch for now to simulate it, I imagine this is very bad + its janky so remove it
	if (!patrollingBehavior.standStill) {
		if (abs(motionComponent.velocity.x) - abs(patrollingBehavior.patrolSpeed) != 0) {
			motionComponent.velocity.x = patrollingBehavior.patrolSpeed;
		} else if (rand() % (int) patrollingBehavior.patrolDistance <= 1) {
			motionComponent.velocity.x = -motionComponent.velocity.x;
		}
	}
	else {
		motionComponent.velocity.x = 0;
	}
}

// NOTE: rn. all enemy speeds are same as player, so they run pretty fast towards the player. maybe should lower that
// it might be fine once the enemies actually respect collision
void AISystem::PathBehavior(Entity enemy_entity) {
	PathingBehavior& enemyPathingBehavior = registry.pathingBehaviors.get(enemy_entity);
	Entity player_entity = registry.players.entities.front();
	TransformComponent& playerTransformComponent = registry.transforms.get(player_entity);
	TransformComponent& enemyTransformComponent = registry.transforms.get(enemy_entity);
	MotionComponent& enemyMotionComponent = registry.motions.get(enemy_entity);
	if (playerTransformComponent.position.x > enemyTransformComponent.position.x) {
		enemyMotionComponent.velocity.x = enemyPathingBehavior.pathSpeed;
	} else {
		enemyMotionComponent.velocity.x = -enemyPathingBehavior.pathSpeed;
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


/*
* this is for actual pathfinding, blocked for now
void AISystem::InitializeAISystem() {
	levelSchematic = new int[NUMTILESWIDE * NUMTILESTALL]; // TODO write something that deletes this in de-initialize etc.
	for (int i = 0; i < NUMTILESWIDE; i++) {
		for (int j = 0; j < NUMTILESTALL; j++) {
			levelSchematic[i * NUMTILESTALL + j] = (levelTiles[i][j] != 0) ? 1 : 0;
		}
	}
}

// Manhattan distance heuristic for heuristic search
float AISystem::Heuristic(vec2 enemyPos, vec2 goalPos) {
	return abs(enemyPos[0] - goalPos[0]) + abs(enemyPos[0] - goalPos[0]);
}

void AISystem::AccessLevelSchematic(float pos) {
	int i = 0;
	j = 0;
	levelSchematic[i * NUMTILESTALL + j];
}
*/

