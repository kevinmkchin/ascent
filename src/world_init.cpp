#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


// TODO(Kevin): for all the entities we want to create, we prob put their initialization code/functions here

Entity createBug(vec2 position)
{
	// Reserve an entity
	auto entity = Entity();

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.rotation = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

    vec2 dimensions = { 64, 64 };

	motion.center = dimensions / 2.f;
    motion.collision_pos = dimensions / 2.f;
    motion.collision_neg = dimensions / 2.f;

    registry.eatables.emplace(entity);
    registry.sprites.insert(
            entity,
            {
                    dimensions,
                    TEXTURE_ASSET_ID::BUG
            }
            );

	return entity;
}

Entity createSpelunkyDude(vec2 position) {
    // Reserve an entity
    auto entity = Entity();

    // Initialize the position, scale, and physics components
    auto& motion = registry.motions.emplace(entity);
    motion.rotation = 0.f;
    motion.velocity = { 0, 0 };
    motion.position = position;

    vec2 dimensions = { 64, 64 };

    motion.center = dimensions / 2.f;
    motion.collision_pos = dimensions / 2.f;
    motion.collision_neg = dimensions / 2.f;

	registry.players.emplace(entity);
	registry.healthBar.emplace(entity);

    registry.sprites.insert(
            entity,
            {
                    dimensions,
                    TEXTURE_ASSET_ID::SPELUNKY
            }
    );

    return entity;
}

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

    registry.eatables.emplace(entity);

	auto& hb = registry.healthBar.emplace(entity);
	hb.health = 0.f;

	//registry.healthBar.emplace(entity);
    registry.sprites.insert(
        entity,
        {
                dimensions,
                TEXTURE_ASSET_ID::BOX
        }
    );

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

	registry.deadlys.emplace(entity);
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
