#include "item_holder_system.hpp"
#include "components.hpp"

INTERNAL float itemGravity = 500.f;
INTERNAL float itemThrowUpwardVelocity = -125.f;
INTERNAL float itemThrowSideVelocity = 150.f;

ItemHolderSystem::ItemHolderSystem()
= default;

INTERNAL void ResolvePickUp(HolderComponent& holderComponent)
{
    if(holderComponent.want_to_pick_up && holderComponent.held_weapon.GetTagAndID() == 0 && holderComponent.near_weapon.GetTagAndID() != 0)
    {
        holderComponent.held_weapon = holderComponent.near_weapon;
        Item& item = registry.items.get(holderComponent.held_weapon);
        item.collidableWithEnvironment = false;
        item.thrown = false;
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
        Item& item = registry.items.get(holderComponent.held_weapon);
        item.collidableWithEnvironment = true;
        item.thrown = true;
        item.grounded = false;

        MotionComponent& motion = registry.motions.get(holderComponent.held_weapon);
        motion.acceleration.y = itemGravity;

        holderComponent.held_weapon = Entity();
    }
}

INTERNAL void ResolveThrow(HolderComponent& holderComponent, MotionComponent& holderMotion)
{
    if(holderComponent.want_to_throw && holderComponent.held_weapon.GetTagAndID() != 0)
    {
        Item& item = registry.items.get(holderComponent.held_weapon);
        item.collidableWithEnvironment = true;
        item.thrown = true;
        item.grounded = false;

        MotionComponent& motion = registry.motions.get(holderComponent.held_weapon);
        motion.acceleration.y = itemGravity;

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
        }
        else
        {
            weaponTransform.position.x = holderTransform.position.x - holderTransform.center.x;
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

        ResolvePickUp(holderComponent);
        ResolveDrop(holderComponent);
        ResolveThrow(holderComponent, holderMotion);
        ResolveItemMovement(holderComponent, holderMotion, holderTransform);
    }
}
