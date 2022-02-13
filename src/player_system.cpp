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
INTERNAL float playerAirAcceleration = 650.f;
INTERNAL float playerAirDeceleration = 500.f;
// JUMP BUFFERING https://twitter.com/MaddyThorson/status/1238338575545978880?s=20&t=iRoDq7J9Um83kDeZYr_dvg
INTERNAL float jumpBufferMaxHoldSeconds = 0.22f;
INTERNAL float jumpBufferMaxTapSeconds = 0.12f;
// COYOTE TIME
INTERNAL float coyoteTimeDefaultSeconds = 0.08f;
// VARIABLE JUMP HEIGHT
INTERNAL float percentYVelocityOnJumpRelease = 0.5f;
// LADDER
INTERNAL float ladderClimbSpeed = 64.f;

INTERNAL bool bPendingJump = false;
INTERNAL bool bJumping = false;
INTERNAL bool bJumpKeyHeld = false;
INTERNAL float jumpBufferTimer = 999.f;
INTERNAL float coyoteTimer = coyoteTimeDefaultSeconds;
INTERNAL bool bLaddered = false;

INTERNAL void HandleBasicMovementInput(MotionComponent& playerMotion)
{
    bool bLeftKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_A) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_LEFT);
    bool bRightKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_D) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_RIGHT);
    bool bJumpKeyJustPressed = Input::HasKeyBeenPressed(SDL_SCANCODE_J) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_A); // @TODO controller bind
    bool bJumpKeyJustReleased = Input::HasKeyBeenReleased(SDL_SCANCODE_J) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_A);
    bJumpKeyHeld = Input::IsKeyPressed(SDL_SCANCODE_J) || Input::GetGamepad(0).IsPressed(GAMEPAD_A);

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

    if(bJumpKeyJustReleased && bJumping && playerMotion.velocity.y < 0.f && !bPendingJump)
    {
        // variable jump height
        // only if we are moving up, and if this is the first time we released jump while jumping
        playerMotion.velocity.y *= percentYVelocityOnJumpRelease;
    }

    playerMotion.acceleration.y = playerGravity;
    playerMotion.terminalVelocity.x = playerMaxMoveSpeed;
    playerMotion.terminalVelocity.y = playerMaxFallSpeed;
}

INTERNAL void ResolveComplexMovement(float deltaTime, MotionComponent& playerMotion)
{
    bool bLeftKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_A) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_LEFT);
    bool bRightKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_D) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_RIGHT);
    bool bUpKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_W) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_UP);
    bool bDownKeyPressed = Input::IsKeyPressed(SDL_SCANCODE_S) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_DOWN);

    bool bGrounded = false;
    bool bStillLaddered = false;
    bool bCollidedDirectlyAbove = false;
    bool bJumpingAndAscending = bJumping && playerMotion.velocity.y < 0.f;

    std::vector<CollisionEvent> playerRelevantCollisions;
    std::vector<CollisionEvent> groundableCollisions;
    std::vector<CollisionEvent> directlyAboveCollisions;

    // Check if grounded or colliding above
    const auto& collisionsRegistry = registry.collisionEvents;
    for (u32 i = 0; i < collisionsRegistry.components.size(); ++i)
    {
        const CollisionEvent colEvent = collisionsRegistry.components[i];
        Entity entity = collisionsRegistry.entities[i];
        Entity entity_other = colEvent.other;

        if(entity.GetTag() != TAG_PLAYER)
        {
            continue;
        }

        if (entity_other.GetTag() == TAG_PLAYERBLOCKABLE)
        {
            playerRelevantCollisions.push_back(colEvent);

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
                    groundableCollisions.push_back(colEvent);
                    bGrounded = true;
                }
                if(collisionCheck.collision_overlap.y > 0.f)
                {
                    directlyAboveCollisions.push_back(colEvent);
                    bCollidedDirectlyAbove = true;
                }
            }
        }

        /** If pressing up, colliding with a ladder, and not jumping and ascending, then we can climb ladder */
        if (bUpKeyPressed && entity_other.GetTag() == TAG_LADDER && !bJumpingAndAscending)
        {
            bLaddered = true;
        }
        if (bLaddered && entity_other.GetTag() == TAG_LADDER)
        {
            bStillLaddered = true;
        }
    }

    if(!bStillLaddered)
    {
        bLaddered = false;
    }

    // Check if actually grounded
    if (bGrounded)
    {
        for (auto gcol : groundableCollisions)
        {
            for (auto pcol : playerRelevantCollisions)
            {
                if (gcol.collision_overlap.x == pcol.collision_overlap.x
                    && gcol.collision_overlap.y != pcol.collision_overlap.y)
                {
                    /* Check we aren't in a wall */
                    bGrounded = false;
                    break;
                }
            }
        }
    }
    // Check if actually hit head
    if (bCollidedDirectlyAbove)
    {
        for (auto acol : directlyAboveCollisions)
        {
            for (auto pcol : playerRelevantCollisions)
            {
                if (acol.collision_overlap.x == pcol.collision_overlap.x
                    && acol.collision_overlap.y != pcol.collision_overlap.y)
                {
                    /* Check we aren't in a wall */
                    bCollidedDirectlyAbove = false;
                    break;
                }
            }
        }
    }
    if(bLaddered)
    {
        playerMotion.acceleration.y = 0.0;
        playerMotion.velocity.y = 0.f;
        if(bUpKeyPressed)
        {
            playerMotion.velocity.y += -ladderClimbSpeed;
        }
        if(bDownKeyPressed)
        {
            playerMotion.velocity.y += ladderClimbSpeed;
        }
        bGrounded = true;
    }

    if(bGrounded)
    {
        bJumping = false;
        if(!bLaddered)
        {
            // Only set velocity to 0 if we are not climbing ladder
            playerMotion.velocity.y = 0.f;
        }
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
            bLaddered = false;
            playerMotion.velocity.y = -playerJumpSpeed;
        }
    }
}

void PlayerSystem::Step(float deltaTime)
{
    const Entity playerEntity = registry.players.entities[0];

    MotionComponent& playerMotion = registry.motions.get(playerEntity);

    HandleBasicMovementInput(playerMotion);
    ResolveComplexMovement(deltaTime, playerMotion);
}