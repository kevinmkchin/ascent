#pragma once

#include "common.hpp"
#include "tiny_ecs_registry.hpp"

class ItemHolderSystem
{
public:
    ItemHolderSystem();

    void Step(float deltaTime);
};
