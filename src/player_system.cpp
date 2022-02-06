#pragma once

#include "player_system.hpp"
#include "physics_system.hpp"

/* PLAYER CONTROLLER CONFIGURATION */
INTERNAL float playerGravity = 150.f;
INTERNAL float playerJumpSpeed = 100.f;
INTERNAL float playerMaxMoveSpeed = 64.f;
INTERNAL float playerMaxFallSpeed = 200.f;
INTERNAL float playerGroundAcceleration = 700.f;
INTERNAL float playerGroundDeceleration = 1000.f;
INTERNAL float playerAirAcceleration = 540.f;
INTERNAL float playerAirDeceleration = 200.f;

INTERNAL bool bPendingJump = false;
INTERNAL bool bJumping = false;

INTERNAL void HandleInput(Motion& playerMotion)
{
    bool bLeftKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_A) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_LEFT);
    bool bRightKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_D) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_RIGHT);
    bool bJumpKeyJustPressed = Input::HasKeyBeenPressed(SDL_SCANCODE_W) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_A); // @TODO controller bind

    float currentXAcceleration = 0.f;

    if(bLeftKeyPressed)
    {
        currentXAcceleration += -(bJumping ? playerAirAcceleration : playerGroundAcceleration);
    }
    else if(playerMotion.velocity.x < -5.f)
    {
        currentXAcceleration += bJumping ? playerAirDeceleration : playerGroundDeceleration;
    }
    else if(playerMotion.velocity.x < 0.f)
    {
        playerMotion.velocity.x = 0.f;
    }

    if(bRightKeyPressed)
    {
        currentXAcceleration += bJumping ? playerAirAcceleration : playerGroundAcceleration;
    }
    else if(playerMotion.velocity.x > 5.f)
    {
        currentXAcceleration += -(bJumping ? playerAirDeceleration : playerGroundDeceleration);
    }
    else if(playerMotion.velocity.x > 0.f)
    {
        playerMotion.velocity.x = 0.f;
    }

    playerMotion.acceleration.x = currentXAcceleration;

    if(bJumpKeyJustPressed && !bJumping)
    {
        bPendingJump = true;
    }

    playerMotion.acceleration.y = playerGravity;
    playerMotion.terminalVelocity.x = playerMaxMoveSpeed;
    playerMotion.terminalVelocity.y = playerMaxFallSpeed;
}

INTERNAL void ResolveMovement(float deltaTime, Motion& playerMotion)
{
    bool bGrounded = false;
    bool bCollidedDirectlyAbove = false;

    //handle jump request + gravity
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
    ResolveMovement(deltaTime, playerMotion);
}