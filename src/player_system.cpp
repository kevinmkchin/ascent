#pragma once

#include "player_system.hpp"
#include "physics_system.hpp"



PlayerSystem::PlayerSystem()
{

}

#pragma region PLAYER_MOVEMENT
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

INTERNAL bool bFaceRight = false; // facing right previously
INTERNAL float time_in_animation = 0.f;
INTERNAL int player_animation_state = 6;

INTERNAL void HandleBasicMovementInput(MotionComponent& playerMotion)
{
    const bool bLeftKeyPressed = Input::GameLeftIsPressed();
    const bool bRightKeyPressed = Input::GameRightIsPressed();
    const bool bJumpKeyJustPressed = Input::GameJumpHasBeenPressed();
    const bool bJumpKeyJustReleased = Input::GameJumpHasBeenReleased();
    bJumpKeyHeld = Input::GameJumpIsPressed();

    float currentXAcceleration = 0.f;

    if(bLeftKeyPressed)
    {
        playerMotion.facingRight = false;
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
        playerMotion.facingRight = true;
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
    const bool bLeftKeyPressed = Input::GameLeftIsPressed();
    const bool bRightKeyPressed = Input::GameRightIsPressed();
    const bool bUpKeyPressed = Input::GameUpIsPressed();
    const bool bDownKeyPressed = Input::GameDownIsPressed();

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

        if(entity_other.GetTag() == TAG_PLAYERBLOCKABLE)
        {
            playerRelevantCollisions.push_back(colEvent);

            Player& player = registry.players.get(entity);
            CollisionComponent& playerCollider = registry.colliders.get(entity);

            // Note(Kevin): this second collision check redundant right now but may become needed later - keep for now?
            CollisionInfo collisionCheck = CheckCollision(playerCollider, registry.colliders.get(entity_other));
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

INTERNAL void HandleBasicInteractionInput(HolderComponent& playerHolder)
{
    const bool bPickUpKeyPressed = Input::GamePickUpIsPressed();
    const bool bDropKeyPressed = Input::GameDropIsPressed();
    const bool bThrowKeyPressed = Input::GameThrowIsPressed();

    playerHolder.want_to_pick_up = bPickUpKeyPressed;
    playerHolder.want_to_drop = bDropKeyPressed;
    playerHolder.want_to_throw = bThrowKeyPressed;
}
#pragma endregion

void PlayerSystem::CheckIfLevelUp()
{
    Player& playerComponent = registry.players.get(playerEntity);
    if(playerComponent.experience > PLAYER_EXP_THRESHOLDS_ARRAY[playerComponent.level])
    {
        ++playerComponent.level;
        printf("LEVEL UP!\n");
    }

    if(Input::IsKeyPressed(SDL_SCANCODE_T))
    {
        playerComponent.experience += 1.f;
    }
}

INTERNAL void HandleSpriteSheetFrame(float deltaTime, MotionComponent& playerMotion, SpriteComponent& playerSprite) {

    /* NOTES:
    Row 1: Death animation
    Row 2: Run animation
    Row 3: Pushing
    Row 4/5: Attack animation
    Row 6: Idle animation
    Row 7: Jump down
    Row 8: Jump up
    .
    .
    .
    etc... more animations
    .
    .
    .
    States:
    1 - Run Left
    2 - Run Right
    3 - Idle Left
    4 - Idle Right
    5 - Jump Down Left
    6 - Jump Down Right
    7 - Jump Up Left
    8 - Jump Up Right

    */

    float x_velocity = playerMotion.velocity.x;
    float y_velocity = playerMotion.velocity.y;

    if (x_velocity > 0.f) {
        bFaceRight = true;
    }
    else if (x_velocity < 0.f) {
        bFaceRight = false;
    }

    int state;
    if (y_velocity != 0.f) {
        if (y_velocity > 0.f) {
            // jump up
            playerSprite.selected_animation = 3;
            state = bFaceRight ? 8 : 7;
            playerSprite.current_frame = (player_animation_state == state) ? playerSprite.current_frame : 0;
        }
        else {
            // jump down
            playerSprite.selected_animation = 2;
            state = bFaceRight ? 6 : 5;
            playerSprite.current_frame = (player_animation_state == state) ? playerSprite.current_frame : 0;
        }
    }
    else if (x_velocity != 0.f) {
        // run
        playerSprite.selected_animation = 1;
        state = bFaceRight ? 2 : 1;
        playerSprite.current_frame = (player_animation_state == state) ? playerSprite.current_frame : 0;
    }
    else {
        // idle
        playerSprite.selected_animation = 0;
        state = bFaceRight ? 4 : 3;
        playerSprite.current_frame = (player_animation_state == state) ? playerSprite.current_frame : 0;
    }

    playerSprite.reverse = bFaceRight ? false : true;
    player_animation_state = state;
}

void PlayerSystem::Step(float deltaTime)
{
    if(registry.players.entities.empty()) { return; }
    playerEntity = registry.players.entities[0];

    Player& playerComponent = registry.players.get(playerEntity);
    MotionComponent& playerMotion = registry.motions.get(playerEntity);
    SpriteComponent& playerSprite = registry.sprites.get(playerEntity);
    TransformComponent& playerTransform = registry.transforms.get(playerEntity);
    HolderComponent& playerHolder = registry.holders.get(playerEntity);

    CheckIfLevelUp();

    HandleBasicMovementInput(playerMotion);
    HandleBasicInteractionInput(playerHolder);
    ResolveComplexMovement(deltaTime, playerMotion);
    HandleSpriteSheetFrame(deltaTime, playerMotion, playerSprite);

}