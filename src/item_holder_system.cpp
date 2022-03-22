#include "item_holder_system.hpp"
#include "components.hpp"

INTERNAL float itemGravity = 500.f;
INTERNAL float itemThrowUpwardVelocity = -125.f;
INTERNAL float itemThrowSideVelocity = 150.f;
INTERNAL float itemShootSideVelocity = 250.f;

ItemHolderSystem::ItemHolderSystem()
= default;

INTERNAL Entity createArrow(vec2 position)
{
    auto entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    auto& motion = registry.motions.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);
    registry.items.emplace(entity);

    transform.position = position;
    transform.rotation = 0.f;
    transform.center = {8, 8};

    collider.collision_pos = vec2(16.f, 8.f) / 2.f;
    collider.collision_neg = vec2(16.f, 8.f) / 2.f;

    float maxFallSpeed = 200.f;
    motion.terminalVelocity.y = maxFallSpeed;

    registry.playerProjectiles.emplace(entity);
    registry.activePlayerProjectiles.emplace(entity);

    registry.sprites.insert(
            entity,
            {
                    {16, 16},
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
                            { // idle
                        1,
                        8,
                        0.f
                            }
                    },
            }
    );

    return entity;
}


INTERNAL void ResolvePickUp(HolderComponent& holderComponent)
{
    if(holderComponent.want_to_pick_up && holderComponent.held_weapon.GetTagAndID() == 0 && holderComponent.near_weapon.GetTagAndID() != 0)
    {
        holderComponent.held_weapon = holderComponent.near_weapon;
        Item& item = registry.items.get(holderComponent.held_weapon);
        item.collidableWithEnvironment = false;
        item.grounded = false;

        MotionComponent& motion = registry.motions.get(holderComponent.held_weapon);
        motion.velocity = {0.f, 0.f};
        motion.acceleration = {0.f, 0.f};
    }
}

INTERNAL void ResolveDrop(HolderComponent& holderComponent)
{
    if(holderComponent.want_to_drop && holderComponent.held_weapon.GetTagAndID() != 0)
    {
        Entity held_weapon = holderComponent.held_weapon;

        Item& item = registry.items.get(held_weapon);
        item.collidableWithEnvironment = true;
        item.grounded = false;

        if ((registry.meleeWeapons.has(held_weapon) || registry.rangedWeapons.has(held_weapon)) && !registry.activePlayerProjectiles.has(held_weapon))
        {
            registry.activePlayerProjectiles.emplace(held_weapon);
        }

        MotionComponent& motion = registry.motions.get(held_weapon);
        motion.acceleration.y = itemGravity;

        holderComponent.held_weapon = Entity();
    }
}

INTERNAL void ResolveThrow(HolderComponent& holderComponent, MotionComponent& holderMotion)
{
    if(holderComponent.want_to_throw && holderComponent.held_weapon.GetTagAndID() != 0)
    {
        Entity held_weapon = holderComponent.held_weapon;

        Item& item = registry.items.get(held_weapon);
        item.collidableWithEnvironment = true;
        item.grounded = false;

        MotionComponent& motion = registry.motions.get(held_weapon);
        motion.acceleration.y = itemGravity;

        if ((registry.meleeWeapons.has(held_weapon) || registry.rangedWeapons.has(held_weapon)) && !registry.activePlayerProjectiles.has(held_weapon))
        {
            registry.activePlayerProjectiles.emplace(held_weapon);
        }

        if(holderMotion.facingRight)
        {
            motion.velocity = {itemThrowSideVelocity, itemThrowUpwardVelocity};
        }
        else
        {
            motion.velocity = {-itemThrowSideVelocity, itemThrowUpwardVelocity};
        }

        holderComponent.held_weapon = Entity();
    }
}

INTERNAL void ResolveShoot(HolderComponent& holderComponent, MotionComponent& holderMotion, TransformComponent& holderTransform)
{
    if(holderComponent.want_to_shoot && holderComponent.held_weapon.GetTagAndID() != 0)
    {
        Entity projectileEntity;
        bool wasProjectileFired = false;

        switch (holderComponent.held_weapon.GetTag()) {
            case (TAG_BOW):
            {
                RangedWeapon& actualBow = registry.rangedWeapons.get(holderComponent.held_weapon);
                if (actualBow.rangedAttackTimeElapsed > actualBow.rangedAttackTimeCooldown) {
                    projectileEntity = createArrow(holderTransform.position);
                    auto& actualProjectile = registry.playerProjectiles.get(projectileEntity);
                    actualProjectile.attackPower = actualBow.attackPower;
                    actualProjectile.attackVariance = actualBow.attackVariance;
                    SpriteComponent& sprite = registry.sprites.get(holderComponent.held_weapon);
                    sprite.selected_animation = 0;
                    sprite.animations[0].played = false;
                    actualBow.rangedAttackTimeElapsed = 0;
                    wasProjectileFired = true;
                }
                break;
            }
            default:
            {
                return;
            }
        }

        if (wasProjectileFired) {
            Item& item = registry.items.get(projectileEntity);
            item.collidableWithEnvironment = true;
            item.grounded = false;
            MotionComponent& motion = registry.motions.get(projectileEntity);
            motion.acceleration.y = itemGravity;

            if (holderMotion.facingRight)
            {
                motion.velocity = { itemShootSideVelocity, itemThrowUpwardVelocity };
                registry.sprites.get(projectileEntity).reverse = false;
            }
            else
            {
                motion.velocity = { -itemShootSideVelocity, itemThrowUpwardVelocity };
                registry.sprites.get(projectileEntity).reverse = true;
            }
        }
    }
}

INTERNAL void ResolveItemMovement(HolderComponent& holderComponent, MotionComponent& holderMotion, TransformComponent& holderTransform)
{
    Entity weapon = holderComponent.held_weapon;
    if(weapon.GetTagAndID() != 0)
    {
        TransformComponent& weaponTransform = registry.transforms.get(weapon);
        weaponTransform.position = holderTransform.position;
        if(holderMotion.facingRight)
        {
            weaponTransform.position.x = holderTransform.position.x + holderTransform.center.x;
            registry.sprites.get(weapon).reverse = false;
        }
        else
        {
            weaponTransform.position.x = holderTransform.position.x - holderTransform.center.x;
            registry.sprites.get(weapon).reverse = true;
        }
    }
}

INTERNAL void ResolveCooldown(HolderComponent holderComponent, float elapsedTime) {
    if (registry.rangedWeapons.has(holderComponent.held_weapon)) {
        registry.rangedWeapons.get(holderComponent.held_weapon).rangedAttackTimeElapsed += elapsedTime;
    }
}

void ItemHolderSystem::Step(float deltaTime)
{
    std::vector<Entity> holders = registry.holders.entities;
    for(Entity holder : holders)
    {
        HolderComponent& holderComponent = registry.holders.get(holder);
        MotionComponent& holderMotion = registry.motions.get(holder);
        TransformComponent& holderTransform = registry.transforms.get(holder);

        ResolvePickUp(holderComponent);
        ResolveDrop(holderComponent);
        ResolveShoot(holderComponent, holderMotion, holderTransform);
        ResolveCooldown(holderComponent, deltaTime * 1000.f);
        ResolveThrow(holderComponent, holderMotion);
        ResolveItemMovement(holderComponent, holderMotion, holderTransform);
    }
}
