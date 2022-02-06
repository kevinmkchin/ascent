#pragma once

#include "player_system.hpp"
#include "physics_system.hpp"

/* PLAYER CONTROLLER CONFIGURATION */
INTERNAL float playerMoveSpeed = 64.f;
INTERNAL float playerJumpSpeed = 100.f;

INTERNAL bool bPendingJump = false;
INTERNAL bool bJumping = false;

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

    if(Input::HasKeyBeenReleased(SDL_SCANCODE_A) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_DPAD_LEFT))
    {
        playerMotion.velocity.x = 0;
    }
    if(Input::HasKeyBeenReleased(SDL_SCANCODE_D) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_DPAD_RIGHT))
    {
        playerMotion.velocity.x = 0;
    }

    bool bJumpKeyPressed = Input::HasKeyBeenPressed(SDL_SCANCODE_W) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_A); // @TODO controller bind

    if(bJumpKeyPressed && !bJumping)
    {
        bPendingJump = true;
    }
}

//handle jump request + gravity
INTERNAL void ResolveJump(float deltaTime, Motion& playerMotion)
{
    bool bGrounded = false;
    bool bCollidedDirectlyAbove = false;

    const auto& collisionsRegistry = registry.collisionEvents;
    for (u32 i = 0; i < collisionsRegistry.components.size(); ++i)
    {
        const CollisionEvent colEvent = collisionsRegistry.components[i];
        Entity entity = collisionsRegistry.entities[i];
        Entity entity_other = colEvent.other;
        if (registry.players.has(entity))
        {
            Player& player = registry.players.get(entity);
            // Note(Kevin): this second collision check redundant right now but may become needed later - keep for now?
            CollisionInfo collisionCheck = CheckCollision(playerMotion, registry.motions.get(entity_other));
            if (collisionCheck.collides && abs(collisionCheck.collision_overlap.y) < abs(collisionCheck.collision_overlap.x))
            {
                if(collisionCheck.collision_overlap.y <= 0.f
                    && playerMotion.velocity.y >= 0.f) // check we are not already moving up otherwise glitches.
                {
                    bGrounded = true;
                }
                if(collisionCheck.collision_overlap.y > 0.f)
                {
                    bCollidedDirectlyAbove = true;
                }
            }
        }
    }

    if(bGrounded)
    {
        bJumping = false;
        playerMotion.velocity.y = 0.f;
    }
    else
    {
        float gravity = 150.f;
        playerMotion.velocity.y += gravity * deltaTime;
    }

    if(bCollidedDirectlyAbove)
    {
        // Check velocity is negative otherwise we can reset velocity to 0 when already falling
        if(playerMotion.velocity.y < 0.f) { playerMotion.velocity.y = 0.f; }
    }

    if(bPendingJump && bGrounded)
    {
        bPendingJump = false;
        bJumping = true;
        playerMotion.velocity.y = -playerJumpSpeed;
    }
}

void PlayerSystem::Step(float deltaTime)
{
    const Entity playerEntity = registry.players.entities[0];

    Motion& playerMotion = registry.motions.get(playerEntity);

    HandleInput(playerMotion);
    ResolveJump(deltaTime, playerMotion);
}