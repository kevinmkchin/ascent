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
	motion.velocity = { 0, 50 };
	motion.position = position;

	registry.eatables.emplace(entity);
    registry.sprites.insert(
            entity,
            {
                    ivec2(128,128),
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

    registry.eatables.emplace(entity);
    registry.sprites.insert(
            entity,
            {
                    ivec2(64, 64),
                    TEXTURE_ASSET_ID::SPELUNKY
            }
    );

    return entity;
}
