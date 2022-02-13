#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


// Entity initialization code

Entity createBox(vec2 position)
{
    // Reserve an entity
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    motion.velocity = { 0, 0 };

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    registry.sprites.insert(
        entity,
        {
                dimensions,
                0,
                TEXTURE_ASSET_ID::BOX
        }
    );

    return entity;
}

Entity createPlayer(vec2 position)
{
    auto entity = Entity::CreateEntity();
    entity.SetTag(TAG_PLAYER);

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    registry.players.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);

    vec2 dimensions = { 12, 14 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    motion.velocity = { 0, 0 };

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    registry.sprites.insert(
        entity,
        {
            dimensions,
            5,
            TEXTURE_ASSET_ID::BOX
        }
    );

    hb.health = 2000.f;

    return entity;
}

Entity createEnemy(vec2 position)
{
	auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    registry.enemy.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    motion.velocity = { 32.f, 0 };

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

	registry.sprites.insert(
		entity,
		{
				dimensions,
                0,
				TEXTURE_ASSET_ID::MONSTER
		}
	);

	return entity;
}
