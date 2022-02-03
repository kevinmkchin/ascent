#include "common.hpp"

#define TILE_SIZE 16

INTERNAL Entity CreateBasicLevelTile(u32 column, u32 row)
{
    auto entity = Entity();

    auto& motion = registry.motions.emplace(entity);
    motion.position = vec2(column * TILE_SIZE, row * TILE_SIZE);
    vec2 dimensions = { TILE_SIZE, TILE_SIZE };

    motion.center = {0.f,0.f};
    motion.collision_neg = {0.f,0.f};
    motion.collision_pos = dimensions;

    registry.eatables.emplace(entity);
    registry.sprites.insert(
            entity,
            {
                dimensions,
                TEXTURE_ASSET_ID::TILE_EXAMPLE
            }
    );

    return entity;
}
