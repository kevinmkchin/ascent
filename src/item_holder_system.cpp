#include "item_holder_system.hpp"
#include "components.hpp"

ItemHolderSystem::ItemHolderSystem()
= default;

INTERNAL void ResolvePickUp(HolderComponent& holderComponent)
{
    if(holderComponent.want_to_pick_up && holderComponent.held_weapon.GetTagAndID() == 0 && holderComponent.near_weapon.GetTagAndID() != 0)
    {
        holderComponent.held_weapon = holderComponent.near_weapon;
        registry.items.get(holderComponent.held_weapon).collidableWithEnvironment = false;
    }
}

INTERNAL void ResolveDrop(HolderComponent& holderComponent)
{
    if(holderComponent.want_to_drop && holderComponent.held_weapon.GetTagAndID() != 0)
    {
        registry.items.get(holderComponent.held_weapon).collidableWithEnvironment = true;
        registry.motions.get(holderComponent.held_weapon).acceleration = vec2({0.f, 500.f});
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
        if(holderMotion.velocity.x > 0)
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
        ResolveItemMovement(holderComponent, holderMotion, holderTransform);
    }
}
