#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "input.hpp"

// struct PlayerInfo
// {
//     vec2 collision_overlap = {0.f, 0.f};
//     bool collides = false;
// };

class PlayerSystem
{
public:
	void handle_buttons(Motion& playerMotion);
    void handle_physics(Motion& playerMotion);
    PlayerSystem() 
    {
    }
};