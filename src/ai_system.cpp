// internal
#include "ai_system.hpp"

static float elapsedTime = 0.0f;

void AISystem::step(float deltaTime)
{
	elapsedTime += deltaTime * 1000.0f;
	if (elapsedTime >= 2000.0f) {
		for (Entity& enemy : registry.enemy.entities) {
			// MotionComponent& enemyMotion = registry.motions.get(enemy);
			// enemyMotion.velocity.x *= -1.f;
			Enemy_attack(enemy);
		}
		elapsedTime = 0.0f;
	}
}

void AISystem::Enemy_attack(Entity enemy_entity) {
	Enemy& enemy = registry.enemy.get(enemy_entity);
	MotionComponent& enemyMotion = registry.motions.get(enemy_entity);
	Entity playerEntity = registry.players.entities.front();
	MotionComponent& playerMotion = registry.motions.get(playerEntity);
	TransformComponent& player_transform = registry.transforms.get(playerEntity);
	TransformComponent& enemy_transform = registry.transforms.get(enemy_entity);
	vec2 diff_distance = player_transform.position - enemy_transform.position;
	if (diff_distance.x < 100 && diff_distance.x > -100 && diff_distance.y > 0){
		float angle = atan2(diff_distance.y, diff_distance.x);
		vec2 velocity = vec2(cos(angle) * enemy.projectile_speed, sin(angle) * enemy.projectile_speed);
		createEnemyProjectile(enemy_transform.position, velocity, enemy_entity);
	}
}
