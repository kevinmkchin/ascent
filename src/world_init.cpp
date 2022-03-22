#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


// Entity initialization code

// NOTE: Do NOT use Entity() constructor to reserve an entity. Use Entity::CreateEntity() instead.

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
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::BOX
        }
    );

    return entity;
}

Entity createPlayer(vec2 position)
{
    auto entity = Entity::CreateEntity(TAG_PLAYER);

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    registry.holders.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    motion.velocity = { 0, 0 };

    collider.collision_pos = { 4, 5 };
    collider.collision_neg = { 4, 7 };

    registry.sprites.insert(
        entity,
        {
            dimensions,
            5,
            EFFECT_ASSET_ID::SPRITE,
            TEXTURE_ASSET_ID::PLAYER,
            true,
            false,
            true,
            128,
            240,
            0,
            0,
            0.f,
            {
                { // idle
                    4,
                    40,
                    100.f * 4.f
                },
                { // run
                    6,
                    8,
                    100.f * 6.f
                },
                { // jump down
                    3,
                    48,
                    100.f * 3.f
                },
                { // jump up
                    3,
                    56,
                    100.f * 3.f
                }
            },
        }
    );

    registry.players.emplace(entity);
    registry.mutations.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);
    auto cb = registry.goldBar.emplace(entity);
    cb.coins = 50.f;
    hb.health = 100.f;
    hb.maxHealth = 100.f;

    return entity;
}

Entity CreateBatEnemy(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);
    auto& visualComponent = registry.visionComponents.emplace(entity);
    auto& pathingBehavior = registry.pathingBehaviors.emplace(entity);
    auto& flyingBehavior = registry.flyingBehaviors.emplace(entity);
    auto& meleeBehavior = registry.meleeBehaviors.emplace(entity);
    hb.health = 20.f;
    registry.enemy.emplace(entity);
    registry.holders.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    float maxMoveSpeed = 128.f;
    pathingBehavior.goalFromPlayer = { 0, 0 };
    pathingBehavior.pathSpeed = maxMoveSpeed;

    visualComponent.sightRadius = 96.f;

    registry.sprites.insert(
            entity,
            {
                    dimensions,
                    0,
                    EFFECT_ASSET_ID::SPRITE,
                    TEXTURE_ASSET_ID::FLYING_ENEMY,
                    true,
                    false,
                    true,
                    64,
                    32,
                    0,
                    0,
                    0.f,
                    {

                            // idle
                            {
                                    4,
                                    0,
                                    100.f * 4.f
                            },

                            // run
                            {
                                    4,
                                    0,
                                    100.f * 4.f
                            },

                            // death
                            {
                                    4,
                                    4,
                                    100.f * 4.f
                            },

                    },
            }
    );

    return entity;
}

Entity CreateKnightEnemy(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);
    auto& visualComponent = registry.visionComponents.emplace(entity);
    auto& pathingBehavior = registry.pathingBehaviors.emplace(entity);
    auto& patrollingBehavior = registry.patrollingBehaviors.emplace(entity);
    auto& walkingBehavior = registry.walkingBehaviors.emplace(entity);
    auto& meleeBehavior = registry.meleeBehaviors.emplace(entity);

    hb.health = 50.f;
    registry.enemy.emplace(entity);
    registry.holders.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    float maxMoveSpeed = 64.f;
    pathingBehavior.goalFromPlayer = { 0, 0 };
    pathingBehavior.pathSpeed = maxMoveSpeed;

    patrollingBehavior.maxPatrolTime = 200.f;
    patrollingBehavior.patrolSpeed = maxMoveSpeed / 4.f;
    patrollingBehavior.standStill = false;

    visualComponent.sightRadius = 64.f;

	registry.sprites.insert(
		entity,
		{
				dimensions,
                0,
                EFFECT_ASSET_ID::SPRITE,
				TEXTURE_ASSET_ID::MONSTER
		}
	);

	return entity;
}

Entity CreateGoblinEnemy(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);
    auto& visualComponent = registry.visionComponents.emplace(entity);
    auto& pathingBehavior = registry.pathingBehaviors.emplace(entity);
    auto& patrollingBehavior = registry.patrollingBehaviors.emplace(entity);
    auto& walkingBehavior = registry.walkingBehaviors.emplace(entity);
    auto& meleeBehavior = registry.meleeBehaviors.emplace(entity);
    hb.health = 50.f;
    registry.enemy.emplace(entity);
    registry.holders.emplace(entity);

    vec2 dimensions = { 16, 16 };
    vec2 collisionDimension = { 12, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_pos = collisionDimension / 2.f;
    collider.collision_neg = collisionDimension / 2.f;

    float maxMoveSpeed = 64.f;
    pathingBehavior.goalFromPlayer = { 0, 0 };
    pathingBehavior.pathSpeed = maxMoveSpeed;

    patrollingBehavior.maxPatrolTime = 500.f;
    patrollingBehavior.patrolSpeed = maxMoveSpeed / 4.f;
    patrollingBehavior.standStill = false;
    patrollingBehavior.currentPatrolTime = (float) (rand() % 200);

    walkingBehavior.stupid = false;

    visualComponent.sightRadius = 48.f;

    registry.sprites.insert(
        entity,
        {
                dimensions,
                0,
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::GOBLIN,
                true,
                false,
                true,
                96,
                80,
                0,
                0,
                0.f,
                {
                    
                    // idle
                    {
                        4,
                        18,
                        100.f * 4.f
                    },
                    
                    // run
                    {
                        6,
                        0,
                        100.f * 6.f
                    },

                    // death
                    {
                        6,
                        6,
                        100.f * 6.f
                    },
                    
                },
        }
    );

    return entity;
}

Entity CreateGoblinBomberEnemy(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);
    auto& visualComponent = registry.visionComponents.emplace(entity);
    auto& pathingBehavior = registry.pathingBehaviors.emplace(entity);
    auto& patrollingBehavior = registry.patrollingBehaviors.emplace(entity);
    auto& walkingBehavior = registry.walkingBehaviors.emplace(entity);
    auto& meleeBehavior = registry.meleeBehaviors.emplace(entity);
    hb.health = 50.f;
    registry.enemy.emplace(entity);
    registry.holders.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    // TODO: Change Goblin Bomber behavior
    float maxMoveSpeed = 64.f;
    pathingBehavior.goalFromPlayer = { 0, 0 };
    pathingBehavior.pathSpeed = maxMoveSpeed;

    patrollingBehavior.maxPatrolTime = 200.f;
    patrollingBehavior.patrolSpeed = maxMoveSpeed / 4.f;
    patrollingBehavior.standStill = false;

    visualComponent.sightRadius = 64.f;

    registry.sprites.insert(
        entity,
        {
                dimensions,
                0,
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::GOBLIN_BOMBER,
                true,
                false,
                true,
                96,
                64,
                0,
                0,
                0.f,
                {

                // idle
                {
                    4,
                    12,
                    100.f * 4.f
                },

                // run (no run so same as idle)
                {
                    4,
                    12,
                    100.f * 4.f
                },

                // death
                {
                    6,
                    6,
                    100.f * 6.f
                },

            },
        }
    );

    return entity;
}

Entity CreateMushroomEnemy(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);
    auto& visualComponent = registry.visionComponents.emplace(entity);
    auto& pathingBehavior = registry.pathingBehaviors.emplace(entity);
    auto& patrollingBehavior = registry.patrollingBehaviors.emplace(entity);
    auto& rangedBehavior = registry.rangedBehaviors.emplace(entity);
    auto& walkingBehavior = registry.walkingBehaviors.emplace(entity);
    auto& meleeBehavior = registry.meleeBehaviors.emplace(entity);
    hb.health = 50.f;
    registry.enemy.emplace(entity);
    registry.holders.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    float maxMoveSpeed = 64.f;
    pathingBehavior.goalFromPlayer = { 0, 0 };
    pathingBehavior.pathSpeed = maxMoveSpeed;

    patrollingBehavior.maxPatrolTime = 200.f;
    patrollingBehavior.patrolSpeed = maxMoveSpeed / 4.f;
    patrollingBehavior.standStill = false;

    rangedBehavior.lobbing = true;

    visualComponent.sightRadius = 64.f;

    registry.sprites.insert(
        entity,
        {
                dimensions,
                0,
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::MUSHROOM,
                true,
                false,
                true,
                128,
                64,
                0,
                0,
                0.f,
                {

                // idle
                {
                    6,
                    8,
                    100.f * 6.f
                },

                // run
                {
                    8,
                    0,
                    100.f * 8.f
                },

                // death
                {
                    6,
                    16,
                    100.f * 6.f
                },

            },  
        }
    );

    return entity;
}

Entity CreateSlimeEnemy(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);
    auto& visualComponent = registry.visionComponents.emplace(entity);
    auto& pathingBehavior = registry.pathingBehaviors.emplace(entity);
    auto& patrollingBehavior = registry.patrollingBehaviors.emplace(entity);
    auto& walkingBehavior = registry.walkingBehaviors.emplace(entity);
    auto& meleeBehavior = registry.meleeBehaviors.emplace(entity);
    hb.health = 50.f;
    registry.enemy.emplace(entity);
    registry.holders.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    float maxMoveSpeed = 64.f;
    pathingBehavior.goalFromPlayer = { 0, 0 };
    pathingBehavior.pathSpeed = maxMoveSpeed;

    patrollingBehavior.maxPatrolTime = 200.f;
    patrollingBehavior.patrolSpeed = maxMoveSpeed / 4.f;
    patrollingBehavior.standStill = false;

    visualComponent.sightRadius = 64.f;

    registry.sprites.insert(
        entity,
        {
                dimensions,
                0,
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::SLIME,
                true,
                true,
                false,
                240,
                80,
                0,
                0,
                0.f,
                {

                // idle
                {
                    5,
                    45,
                    100.f * 5.f
                },

                // run
                {
                    15,
                    15,
                    50.f * 15.f
                },

                // death
                {
                    6,
                    30,
                    100.f * 6.f
                },

            },
        }
    );

    return entity;
}

Entity CreateWormEnemy(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& hb = registry.healthBar.emplace(entity);
    auto& visualComponent = registry.visionComponents.emplace(entity);
    auto& pathingBehavior = registry.pathingBehaviors.emplace(entity);
    auto& patrollingBehavior = registry.patrollingBehaviors.emplace(entity);
    auto& walkingBehavior = registry.walkingBehaviors.emplace(entity);
    auto& meleeBehavior = registry.meleeBehaviors.emplace(entity);
    hb.health = 50.f;
    registry.enemy.emplace(entity);
    registry.holders.emplace(entity);

    vec2 dimensions = { 16, 8 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    float maxMoveSpeed = 64.f;
    pathingBehavior.goalFromPlayer = { 0, 0 };
    pathingBehavior.pathSpeed = maxMoveSpeed;

    patrollingBehavior.maxPatrolTime = 200.f;
    patrollingBehavior.patrolSpeed = maxMoveSpeed / 4.f;
    patrollingBehavior.standStill = false;

    visualComponent.sightRadius = 64.f;

    registry.sprites.insert(
        entity,
        {
                dimensions,
                0,
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::WORM,
                true,
                false,
                true,
                96,
                24,
                0,
                0,
                0.f,
                {

                // idle
                {
                    6,
                    0,
                    100.f * 6.f
                },

                // run
                {
                    6,
                    0,
                    100.f * 6.f
                },

                // death
                {
                    6,
                    6,
                    100.f * 6.f
                },

            },
        }
    );

    return entity;
}

Entity createSword(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    registry.weapons.emplace(entity);
    registry.items.emplace(entity);

    vec2 dimensions = { 15, 20 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_neg = { 6, 8 };
    collider.collision_pos = { 6, 7 };

    float maxFallSpeed = 200.f;
    motion.terminalVelocity.y = maxFallSpeed;

    registry.sprites.insert(
            entity,
            {
                    dimensions,
                    15,
                    EFFECT_ASSET_ID::SPRITE,
                    TEXTURE_ASSET_ID::SWORD
            }
    );

    return entity;
}

Entity createBow(vec2 position)
{
    auto entity = Entity::CreateEntity(TAG_BOW);

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    registry.items.emplace(entity);

    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;

    float maxFallSpeed = 200.f;
    motion.terminalVelocity.y = maxFallSpeed;

    registry.sprites.insert(
            entity,
            {
                    dimensions,
                    15,
                    EFFECT_ASSET_ID::SPRITE,
                    TEXTURE_ASSET_ID::BOW_AND_ARROW,
                    true,
                    false,
                    true,
                    48,
                    48,
                    0,
                    0,
                    0.f,
                    {
//                        { // idle
//                            1,
//                            0,
//                            0
//                        },
                        {
                            3,
                            0,
                            300.f,
                            false,
                            true
                        }
                    },
            }
    );

    return entity;
}

Entity createEnemyLobbingProjectile(vec2 position, vec2 velocity, vec2 acceleration, Entity enemy) {
    auto entity = Entity::CreateEntity();
    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& rangedBehavior = registry.rangedBehaviors.get(enemy);
    vec2 dimensions = { 8, 8 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    motion.velocity = velocity;
    motion.acceleration = acceleration;
    motion.terminalVelocity.y = 200.f;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;
    auto& projectile = registry.enemyProjectiles.emplace(entity);
    projectile.attackPower = rangedBehavior.attackPower;
    projectile.enemy_projectile = enemy;
    registry.sprites.insert(
        entity,
        {
                dimensions,
                10,
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::LOB_PROJECTILE
        }
    );

    return entity;
}

Entity createEnemyProjectile(vec2 position, vec2 velocity, Entity enemy) {
    auto entity = Entity::CreateEntity();
    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    auto& rangedBehavior = registry.rangedBehaviors.get(enemy);
    vec2 dimensions = { 16, 16 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    motion.velocity = velocity;

    collider.collision_pos = dimensions / 2.f;
    collider.collision_neg = dimensions / 2.f;
    auto& projectile = registry.enemyProjectiles.emplace(entity);
    projectile.attackPower = rangedBehavior.attackPower;
    projectile.enemy_projectile = enemy;
    registry.sprites.insert(
        entity,
        {
                dimensions,
                10,
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::FIRE
        }
    );

    return entity;
}

Entity CreateShopItem(vec2 position, u8 shopItemIndex) // remove later - just a temporary way of creating shop items
{
    Entity entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    transform.position = position;
    transform.center = {0.f,0.f};

    registry.sprites.insert(
            entity,
            {
                { 16, 16 },
                0,
                EFFECT_ASSET_ID::SPRITE,
                TEXTURE_ASSET_ID::ASCENT_MUTATIONS_SHEET,
                true, false, true, 128, 128,
                0,
                0,
                0.f,
                {
                    {
                        1,
                        shopItemIndex,
                        0.f
                    }
                }
            }
    );

    auto& collider = registry.colliders.emplace(entity);
    collider.collision_neg = { 0, 0 };
    collider.collision_pos = { 16, 16 };
    collider.collider_position = transform.position;

    return entity;
}


Entity createExp(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    registry.exp.emplace(entity);

    vec2 dimensions = { 8, 8 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_neg = { 6, 8 };
    collider.collision_pos = { 6, 7 };

    float maxFallSpeed = 200.f;
    motion.terminalVelocity.y = maxFallSpeed;

    motion.velocity.x = (float) RandomInt(-150, 150);
    motion.velocity.y = (float) RandomInt(-160, -80);
    motion.drag.x = 300.f;
    motion.acceleration.y = 320.f;

    registry.sprites.insert(
        entity,
        {
            dimensions,
            15,
            EFFECT_ASSET_ID::SPRITE,
            TEXTURE_ASSET_ID::EXP,
            true,
            false,
            true,
            48,
            8,
            0,
            0,
            0.f,
            {
                {
                    6,
                    0,
                    600.f,
                    true,
                    false
                }
            },
        }
    );

    return entity;
}


Entity createCoins(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    registry.coins.emplace(entity);

    vec2 dimensions = { 8, 8 };
    transform.position = position;
    transform.rotation = 0.f;
    transform.center = dimensions / 2.f;

    collider.collision_neg = { 6, 8 };
    collider.collision_pos = { 6, 7 };

    float maxFallSpeed = 200.f;
    motion.terminalVelocity.y = maxFallSpeed;

    motion.velocity.x = (float) RandomInt(-150, 150);
    motion.velocity.y = (float) RandomInt(-160, -80);
    motion.drag.x = 300.f;
    motion.acceleration.y = 320.f;

    registry.sprites.insert(
        entity,
        {
            dimensions,
            15,
            EFFECT_ASSET_ID::SPRITE,
            TEXTURE_ASSET_ID::COIN,
            true,
            false,
            true,
            48,
            8,
            0,
            0,
            0.f,
            {
                {
                    6,
                    0,
                    600.f,
                    true,
                    false
                }
            },
        }
    );

    return entity;
}