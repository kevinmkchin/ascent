#pragma once

#include "player_system.hpp"
#include "physics_system.hpp"


INTERNAL float playerMoveSpeed = 64.f;
INTERNAL float playerJumpSpeed = 48.f;

INTERNAL void HandleInput(Motion& playerMotion)
{
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
        playerMotion.velocity.y = -playerJumpSpeed;
    }

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
        playerMotion.velocity.y = 0;
    }
}

INTERNAL void ResolveJump(Motion& playerMotion)
{
    //handle jump request + gravity
    auto& collisionsRegistry = registry.collisions;
    bool touchingFloor = false;
    float gravity = 1.f;
    for (u32 i = 0; i < collisionsRegistry.components.size(); ++i)
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
            if (collisionCheck.collides && abs(collisionCheck.collision_overlap.y) < abs(collisionCheck.collision_overlap.x) && collisionCheck.collision_overlap.y <= 0)
            {
                touchingFloor = true;
            }
        }
    }
    if (!touchingFloor)
    {
        playerMotion.velocity.y += gravity;
    }
}

void PlayerSystem::Step(float deltaTime)
{
    const Entity playerEntity = registry.players.entities[0];

    Motion& playerMotion = registry.motions.get(playerEntity);

    HandleInput(playerMotion);
    ResolveJump(playerMotion);
}