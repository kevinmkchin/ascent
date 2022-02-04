// internal
#include "ai_system.hpp"

static float elapsedTime = 0.0f;

void AISystem::step(float deltaTime)
{
	elapsedTime += deltaTime * 1000.0f;
	if (elapsedTime >= 2000.0f) {

		for (Entity& enemy : registry.enemy.entities) {
			Motion& enemyMotion = registry.motions.get(enemy);
			enemyMotion.velocity.x *= -1.f;
		}
		elapsedTime = 0.0f;

	}
}
