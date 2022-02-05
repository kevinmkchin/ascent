#pragma once

#include "player_system.hpp"
#include "physics_system.hpp"

// go through movement requests for player
void PlayerSystem::handle_buttons(Motion& playerMotion) 
{
    float playerMoveSpeed = 64.f;
    float playerJumpSpeed = 48.f;

    if(Input::IsKeyPressed(SDL_SCANCODE_A) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_LEFT))
    {
        playerMotion.velocity.x = -playerMoveSpeed;
    }
    if(Input::IsKeyPressed(SDL_SCANCODE_D) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_RIGHT))
    {
        playerMotion.velocity.x = playerMoveSpeed;
    }
    if(Input::IsKeyPressed(SDL_SCANCODE_W) || Input::GetGamepad(0).IsPressed(GAMEPAD_A)) // @TODO controller bind
    {
        playerMotion.jumpRequest = playerJumpSpeed;
        printf("requesting a jump'\n'");
    }
    // if(Input::GetGamepad(0).isConnected)
    // {
    //     playerMotion.velocity = Input::GetGamepad(0).leftThumbStickDir * playerMoveSpeed;
    // }

    if (Input::HasKeyBeenReleased(SDL_SCANCODE_A) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_DPAD_LEFT))
    {
        playerMotion.velocity.x = 0;
    }
    if (Input::HasKeyBeenReleased(SDL_SCANCODE_D) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_DPAD_RIGHT))
    {
        playerMotion.velocity.x = 0;
    }
    if (Input::HasKeyBeenReleased(SDL_SCANCODE_W) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_A))
    {
        playerMotion.jumpRequest = 0;
    }
}

//handle jump request + gravity
void PlayerSystem::handle_physics(Motion& playerMotion)
{
    auto& collisionsRegistry = registry.collisions;
    bool touchingFloor = false;
    float gravity = 1.f;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) 
    {
		// The entity and its collider
        const Collision colEvent = collisionsRegistry.components[i];
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = colEvent.other;
		if (registry.players.has(entity)) 
        {
            Player& player = registry.players.get(entity);
            Motion& playerMotion = registry.motions.get(entity);
            CollisionInfo collisionCheck = CheckCollision(playerMotion, registry.motions.get(entity_other));
            if (collisionCheck.collides && collisionCheck.collision_overlap.y <= 0) 
            {
                touchingFloor = true;
                printf("touchingFloor'\n'");
            }
        }
    }
    if (!touchingFloor) 
    {
        playerMotion.velocity.y += gravity;
    } else {
        playerMotion.velocity.y = -playerMotion.jumpRequest;
        printf("jumping'\n'");
    }
}