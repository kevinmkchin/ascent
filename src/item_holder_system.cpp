#include "item_holder_system.hpp"
#include "components.hpp"

INTERNAL float itemGravity = 500.f;
INTERNAL float itemThrowUpwardVelocity = -125.f;
INTERNAL float itemThrowSideVelocity = 150.f;
INTERNAL float itemShootSideVelocity = 250.f;

INTERNAL float bowCooldown = 0.5;

ItemHolderSystem::ItemHolderSystem()
= default;

INTERNAL void makeItemDisappear(Entity item)
{
    registry.transforms.get(item).position = {0, 0};
}

INTERNAL void addToInventory(HolderComponent& holderComponent, Entity itemToPickUp)
{
    for (Entity item : holderComponent.carried_items)
    {
        if (item.GetTag() == itemToPickUp.GetTag())
        {
            return;
        }
    }

    if (holderComponent.current_item >= 0)
    {
        makeItemDisappear(holderComponent.carried_items.at(holderComponent.current_item));
    }

    holderComponent.carried_items.push_back(itemToPickUp);
    holderComponent.current_item++;
}


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
    if(holderComponent.want_to_pick_up && holderComponent.near_weapon.GetTagAndID() != 0 && (registry.items.has(holderComponent.near_weapon) || registry.weapons.has(holderComponent.near_weapon)))
    {
        addToInventory(holderComponent, holderComponent.near_weapon);

        Entity held_weapon = holderComponent.carried_items.at(holderComponent.current_item);

        Item& item = registry.items.get(held_weapon);
        item.collidableWithEnvironment = false;
        item.grounded = false;

        MotionComponent& motion = registry.motions.get(held_weapon);
        motion.velocity = {0.f, 0.f};
        motion.acceleration = {0.f, 0.f};
    }
}

INTERNAL void ResolveDrop(HolderComponent& holderComponent)
{
    if (holderComponent.current_item < 0) {
        return;
    }

    if(holderComponent.want_to_drop)
    {
        Entity held_weapon = holderComponent.carried_items.at(holderComponent.current_item);

        Item& item = registry.items.get(held_weapon);
        item.collidableWithEnvironment = true;
        item.grounded = false;

        if (registry.weapons.has(held_weapon) && !registry.activePlayerProjectiles.has(held_weapon))
        {
            registry.activePlayerProjectiles.emplace(held_weapon);
        }

        MotionComponent& motion = registry.motions.get(held_weapon);
        motion.acceleration.y = itemGravity;

        holderComponent.carried_items.erase(holderComponent.carried_items.begin() + holderComponent.current_item);
        holderComponent.current_item--;
    }
}

INTERNAL void ResolveThrow(HolderComponent& holderComponent, MotionComponent& holderMotion)
{
    if (holderComponent.current_item < 0) {
        return;
    }

    if(holderComponent.want_to_throw && holderComponent.near_weapon.GetTagAndID() == 0)
    {
        Entity held_weapon = holderComponent.carried_items.at(holderComponent.current_item);

        Item& item = registry.items.get(held_weapon);
        item.collidableWithEnvironment = true;
        item.grounded = false;

        MotionComponent& motion = registry.motions.get(held_weapon);
        motion.acceleration.y = itemGravity;

        if (registry.weapons.has(held_weapon) && !registry.activePlayerProjectiles.has(held_weapon))
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

        holderComponent.carried_items.erase(holderComponent.carried_items.begin() + holderComponent.current_item);
        holderComponent.current_item--;
    }
}

INTERNAL void ResolveShoot(HolderComponent& holderComponent, MotionComponent& holderMotion, TransformComponent& holderTransform)
{
    if (holderComponent.current_item < 0) {
        return;
    }

    Entity held_weapon = holderComponent.carried_items.at(holderComponent.current_item);

    if(holderComponent.want_to_shoot && registry.weapons.has(held_weapon))
    {
        Weapon& weapon = registry.weapons.get(held_weapon);

        if (weapon.cooldown > 0)
        {
            return;
        }

        weapon.cooldown = bowCooldown;

        Entity projectile;

        switch (held_weapon.GetTag()) {
            case (TAG_BOW):
            {
                projectile = createArrow(holderTransform.position);
                SpriteComponent& sprite = registry.sprites.get(held_weapon);
                sprite.selected_animation = 0;
                sprite.animations[0].played = false;
                break;
            }
            default:
            {
                return;
            }
        }

        Item& item = registry.items.get(projectile);
        item.collidableWithEnvironment = true;
        item.grounded = false;

        MotionComponent& motion = registry.motions.get(projectile);
        motion.acceleration.y = itemGravity;

        if(holderMotion.facingRight)
        {
            motion.velocity = {itemShootSideVelocity, itemThrowUpwardVelocity};
            registry.sprites.get(projectile).reverse = false;
        }
        else
        {
            motion.velocity = {-itemShootSideVelocity, itemThrowUpwardVelocity};
            registry.sprites.get(projectile).reverse = true;
        }
    }
}

INTERNAL void ResolveItemMovement(HolderComponent& holderComponent, MotionComponent& holderMotion, TransformComponent& holderTransform)
{
    if (holderComponent.current_item < 0) {
        return;
    }

    Entity weapon = holderComponent.carried_items.at(holderComponent.current_item);
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

INTERNAL void ResolveCycle(HolderComponent& holderComponent)
{
    if (holderComponent.current_item < 0 || holderComponent.carried_items.size() < 2) {
        return;
    }

    if (holderComponent.want_to_cycle_right) {
        makeItemDisappear(holderComponent.carried_items.at(holderComponent.current_item));
        holderComponent.current_item = (holderComponent.current_item + 1) % holderComponent.carried_items.size();
    } else if (holderComponent.want_to_cycle_left) {
        makeItemDisappear(holderComponent.carried_items.at(holderComponent.current_item));
        holderComponent.current_item -= 1;
        if (holderComponent.current_item < 0) {
            holderComponent.current_item = holderComponent.carried_items.size() - 1;
        }
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

        if (holderComponent.current_item >= 0)
        {
            Entity held_weapon = holderComponent.carried_items.at(holderComponent.current_item);

            if (registry.weapons.has(held_weapon))
            {
                Weapon& weapon = registry.weapons.get(held_weapon);
                weapon.cooldown -= deltaTime;
            }
        }

        ResolvePickUp(holderComponent);
//        ResolveDrop(holderComponent);
//        ResolveThrow(holderComponent, holderMotion);
        ResolveShoot(holderComponent, holderMotion, holderTransform);
        ResolveCycle(holderComponent);
        ResolveItemMovement(holderComponent, holderMotion, holderTransform);
    }
}
