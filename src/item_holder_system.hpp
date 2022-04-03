#pragma once

#include "common.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_system.hpp"

class ItemHolderSystem
{
public:
    ItemHolderSystem();

    void Step(float deltaTime);

    void Init(WorldSystem *world_sys_arg);

private:
    void ResolveShoot(HolderComponent& holderComponent, MotionComponent& holderMotion, TransformComponent& holderTransform);

    WorldSystem* world;
};
