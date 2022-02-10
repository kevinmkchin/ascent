#pragma once

#include "player_system.hpp"
#include "physics_system.hpp"

/* PLAYER CONTROLLER CONFIGURATION */
INTERNAL float playerGravity = 500.f;
INTERNAL float playerJumpSpeed = 200.f;
INTERNAL float playerMaxMoveSpeed = 64.f;
INTERNAL float playerMaxFallSpeed = 200.f;
INTERNAL float playerGroundAcceleration = 700.f;
INTERNAL float playerGroundDeceleration = 1000.f;
INTERNAL float playerAirAcceleration = 540.f;
INTERNAL float playerAirDeceleration = 200.f;
// JUMP BUFFERING https://twitter.com/MaddyThorson/status/1238338575545978880?s=20&t=iRoDq7J9Um83kDeZYr_dvg
INTERNAL float jumpBufferMaxHoldSeconds = 0.22f;
INTERNAL float jumpBufferMaxTapSeconds = 0.12f;
// COYOTE TIME
INTERNAL float coyoteTimeDefaultSeconds = 0.08f;

INTERNAL bool bPendingJump = false;
INTERNAL bool bJumping = false;
INTERNAL bool bJumpKeyHeld = false;
INTERNAL float jumpBufferTimer = 999.f;
INTERNAL float coyoteTimer = coyoteTimeDefaultSeconds;

INTERNAL void HandleInput(MotionComponent& playerMotion)
{
    bool bLeftKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_A) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_LEFT);
    bool bRightKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_D) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_RIGHT);
    bool bJumpKeyJustPressed = Input::HasKeyBeenPressed(SDL_SCANCODE_W) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_A); // @TODO controller bind
    bJumpKeyHeld = Input::IsKeyPressed(SDL_SCANCODE_W) || Input::GetGamepad(0).IsPressed(GAMEPAD_A);

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

    if(bJumpKeyJustPressed)
    {
        bPendingJump = true;
        jumpBufferTimer = 0.f;
    }

    playerMotion.acceleration.y = playerGravity;
    playerMotion.terminalVelocity.x = playerMaxMoveSpeed;
    playerMotion.terminalVelocity.y = playerMaxFallSpeed;
}

INTERNAL void ResolveMovement(float deltaTime, MotionComponent& playerMotion)
{
    bool bGrounded = false;
    bool bCollidedDirectlyAbove = false;

    // Check if grounded or colliding above
    const auto& collisionsRegistry = registry.collisionEvents;
    for (u32 i = 0; i < collisionsRegistry.components.size(); ++i)
    {
        const CollisionEvent colEvent = collisionsRegistry.components[i];
        Entity entity = collisionsRegistry.entities[i];
        Entity entity_other = colEvent.other;
        if (registry.players.has(entity))
        {
            Player& player = registry.players.get(entity);
            TransformComponent& playerTransform = registry.transforms.get(entity);
            CollisionComponent& playerCollider = registry.colliders.get(entity);

            // Note(Kevin): this second collision check redundant right now but may become needed later - keep for now?
            CollisionInfo collisionCheck = CheckCollision(playerTransform, playerCollider,
                registry.transforms.get(entity_other), registry.colliders.get(entity_other));
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
        coyoteTimer = coyoteTimeDefaultSeconds;
    }
    else if(!bJumping) // if not grounded and not jumping, then we must be falling
    {
        coyoteTimer -= deltaTime;
    }

    if(bCollidedDirectlyAbove)
    {
        // Check velocity is negative otherwise we can reset velocity to 0 when already falling
        if(playerMotion.velocity.y < 0.f) { playerMotion.velocity.y = 0.f; }
    }

    if (bPendingJump)
    {
        // Jump buffering
        jumpBufferTimer += deltaTime; // tick the jump buffer timer
        if (!bJumpKeyHeld && jumpBufferTimer > jumpBufferMaxTapSeconds) { bPendingJump = false; }
        if (jumpBufferTimer > jumpBufferMaxHoldSeconds) { bPendingJump = false; }

        // Actually jump
        if(!bCollidedDirectlyAbove && (bGrounded || (coyoteTimer > 0.f && !bJumping)))
        {
            bPendingJump = false;
            bJumping = true;
            playerMotion.velocity.y = -playerJumpSpeed;
        }
    }
}

void PlayerSystem::Step(float deltaTime)
{
    const Entity playerEntity = registry.players.entities[0];

    MotionComponent& playerMotion = registry.motions.get(playerEntity);

    HandleInput(playerMotion);
    ResolveMovement(deltaTime, playerMotion);
}