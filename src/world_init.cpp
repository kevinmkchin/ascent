#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


// Entity initialization code

Entity createBox(vec2 position) {
    // Reserve an entity
    auto entity = Entity();

    // Initialize the position, scale, and physics components
    auto& motion = registry.motions.emplace(entity);
    motion.rotation = 0.f;
    motion.velocity = { 0, 0 };
    motion.position = position;

    vec2 dimensions = { 16, 16 };

    motion.center = dimensions / 2.f;
    motion.collision_pos = dimensions / 2.f;
    motion.collision_neg = dimensions / 2.f;

    registry.sprites.insert(
        entity,
        {
                dimensions,
                TEXTURE_ASSET_ID::BOX
        }
    );

    return entity;
}

Entity createPlayer(vec2 position)
{
    auto entity = Entity();

    auto& motion = registry.motions.emplace(entity);
    motion.rotation = 0.f;
    motion.velocity = { 0, 0 };
    motion.position = position;

    vec2 dimensions = { 12, 14 };

    motion.center = dimensions / 2.f;
    motion.collision_pos = dimensions / 2.f;
    motion.collision_neg = dimensions / 2.f;

    registry.sprites.insert(
        entity,
        {
            dimensions,
            TEXTURE_ASSET_ID::BOX
        }
    );

    auto& hb = registry.healthBar.emplace(entity);
    hb.health = 2000.f;

    registry.players.emplace(entity);

    return entity;
}

Entity createEnemy(vec2 position) {
	// Reserve an entity
	auto entity = Entity();

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.rotation = 0.f;
	motion.velocity = { 32.f, 0 };
	motion.position = position;

	vec2 dimensions = { 16, 16 };

	motion.center = dimensions / 2.f;
	motion.collision_pos = dimensions / 2.f;
	motion.collision_neg = dimensions / 2.f;

	registry.enemy.emplace(entity);

	registry.sprites.insert(
		entity,
		{
				dimensions,
				TEXTURE_ASSET_ID::MONSTER
		}
	);

	return entity;
}
