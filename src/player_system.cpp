#pragma once

#include "player_system.hpp"
#include "physics_system.hpp"
#include "world_system.hpp"
#include "ui_system.hpp"


PlayerSystem::PlayerSystem()
{

}


void PlayerSystem::Init(WorldSystem* world_sys_arg, UISystem* ui_sys_arg)
{
    world = world_sys_arg;
    ui = ui_sys_arg;
}

#pragma region PLAYER_MOVEMENT
/* PLAYER CONTROLLER CONFIGURATION */
INTERNAL float playerGravity = 500.f;
INTERNAL float playerJumpSpeed = 200.f;
// NO LONGER IN USE!!! movementSpeed comes from PlayerComponent!!! INTERNAL float playerMaxMoveSpeed = 64.f;
INTERNAL float playerMaxFallSpeed = 200.f;
INTERNAL float playerGroundAcceleration = 700.f;
INTERNAL float playerGroundDeceleration = 1000.f;
INTERNAL float playerAirAcceleration = 650.f;
INTERNAL float playerAirDeceleration = 300.f;
// JUMP BUFFERING https://twitter.com/MaddyThorson/status/1238338575545978880?s=20&t=iRoDq7J9Um83kDeZYr_dvg
INTERNAL float jumpBufferMaxHoldSeconds = 0.22f;
INTERNAL float jumpBufferMaxTapSeconds = 0.12f;
// COYOTE TIME
INTERNAL float coyoteTimeDefaultSeconds = 0.09f;
// VARIABLE JUMP HEIGHT
INTERNAL float percentYVelocityOnJumpRelease = 0.6f;
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

INTERNAL void HandleBasicMovementInput(MotionComponent& playerMotion, Player& playerComponent)
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
    playerMotion.terminalVelocity.x = playerComponent.movementSpeed;
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

INTERNAL void HandleItemInteractionInput(HolderComponent& playerHolder)
{   
    const bool bThrowKeyPressed = playerHolder.held_weapon != 0 ? Input::GamePickUpHasBeenPressed() : false;
    const bool bPickUpKeyPressed = playerHolder.held_weapon == 0 ? Input::GamePickUpHasBeenPressed() : false;
    const bool bDropKeyPressed = playerHolder.held_weapon != 0 && Input::GamePickUpHasBeenPressed() && Input::GameDownIsPressed();

    playerHolder.want_to_pick_up = bPickUpKeyPressed;
    playerHolder.want_to_drop = bDropKeyPressed;
    playerHolder.want_to_throw = bThrowKeyPressed;
}
#pragma endregion

void PlayerSystem::CheckIfLevelUp()
{
    if(playerComponentPtr->experience > PLAYER_EXP_THRESHOLDS_ARRAY[playerComponentPtr->level])
    {
        ++(playerComponentPtr->level);
        bLeveledUpLastFrame = true;
        printf("LEVEL UP!\n");
        if(Mix_PlayChannel(-1, world->player_levelup_sound, 0) == -1) 
        {
            printf("Mix_PlayChannel: %s\n",Mix_GetError());
        }
    }

    if(Input::IsKeyPressed(SDL_SCANCODE_T))
    {
        playerComponentPtr->experience += 1.f;
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

    if (Input::GameLeftIsPressed()) {
        bFaceRight = false;
    }
    else if (Input::GameRightIsPressed()) {
        bFaceRight = true;
    }

    int state;
    if (y_velocity != 0.f) {
        if (bJumping && y_velocity > 0.f) {
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

void PlayerSystem::PlayerAttackPrePhysicsStep(float deltaTime)
{
    TransformComponent& playerTransform = registry.transforms.get(playerEntity);

    if(playerMeleeAttackCooldownTimer > 0.f)
    {
        playerMeleeAttackCooldownTimer -= deltaTime;
    }

    if(playerMeleeAttackEntity != 0)
    {
        playerMeleeAttackLengthTimer -= deltaTime;
        auto& transform = registry.transforms.get(playerMeleeAttackEntity);
        // WE DELETE COLLIDER RIGHT FIRST FRAME auto& collider = registry.colliders.get(playerMeleeAttackEntity);
        transform.position = playerTransform.position + playerMeleeAttackPositionOffsetFromPlayer;
        // WE DELETE COLLIDER RIGHT FIRST FRAME collider.collider_position = transform.position;
        if(playerMeleeAttackLengthTimer <= 0.f)
        {
            registry.remove_all_components_of(playerMeleeAttackEntity);
            playerMeleeAttackEntity = Entity();
        }
    }

    if(playerMeleeAttackCooldownTimer <= 0.f && Input::GameAttackHasBeenPressed())
    {
        u8 attackDir = bFaceRight; // 0 left, 1 right, 2 up, 3 down

        if(Input::GameDownIsPressed())
        {
            attackDir = 3;
        }
        if(Input::GameUpIsPressed())
        {
            attackDir = 2;
        }
        if(Input::GameLeftIsPressed())
        {
            attackDir = 0;
        }
        if(Input::GameRightIsPressed())
        {
            attackDir = 1;
        }

        if(attackDir == 3 && !bJumping)
        {
            return; // can only attack down if jumping
        }

        lastAttackDirection = attackDir;

        playerMeleeAttackCooldownTimer = playerComponentPtr->meleeAttackCooldown; // reset timer
        playerMeleeAttackLengthTimer = 0.03f;     // reset timer

        playerMeleeAttackEntity = Entity::CreateEntity(TAG_PLAYERMELEEATTACK);
        auto& transform = registry.transforms.emplace(playerMeleeAttackEntity);
        auto& collider = registry.colliders.emplace(playerMeleeAttackEntity);

        i16 meleeBoxWidth = playerComponentPtr->meleeAttackRange;
        i16 meleeBoxHeight = playerComponentPtr->meleeAttackArc;

        vec2 dimensions;
        if(attackDir == 0)
        {
            transform.position.x = playerTransform.position.x - 8;
            transform.position.y = playerTransform.position.y;
            dimensions = { meleeBoxWidth, meleeBoxHeight };
            transform.center = { meleeBoxWidth, meleeBoxHeight/2 };
            collider.collider_position = transform.position;
            collider.collision_pos = { 0, (i16)(meleeBoxHeight/2) };
            collider.collision_neg = { meleeBoxWidth, (i16)(meleeBoxHeight/2) };
        }
        else if(attackDir == 1)
        {
            transform.position.x = playerTransform.position.x + 8;
            transform.position.y = playerTransform.position.y;
            dimensions = { meleeBoxWidth, meleeBoxHeight };
            transform.center = { 0, meleeBoxHeight/2 };
            collider.collider_position = transform.position;
            collider.collision_pos = { meleeBoxWidth, (i16)(meleeBoxHeight/2) };
            collider.collision_neg = { 0, (i16)(meleeBoxHeight/2) };
        }
        else if(attackDir == 2)
        {
            transform.position.x = playerTransform.position.x;
            transform.position.y = playerTransform.position.y - 11;
            dimensions = { meleeBoxHeight, meleeBoxWidth };
            transform.center = { meleeBoxHeight/2, meleeBoxWidth };
            collider.collider_position = transform.position;
            collider.collision_pos = { (i16)(meleeBoxHeight/2), 0 };
            collider.collision_neg = { (i16)(meleeBoxHeight/2), meleeBoxWidth };
        }
        else if(attackDir == 3)
        {
            transform.position.x = playerTransform.position.x;
            transform.position.y = playerTransform.position.y + 11;
            dimensions = { meleeBoxHeight, meleeBoxWidth };
            transform.center = { meleeBoxHeight/2, 0 };
            collider.collider_position = transform.position;
            collider.collision_pos = { (i16)(meleeBoxHeight/2), meleeBoxWidth };
            collider.collision_neg = { (i16)(meleeBoxHeight/2), 0 };
        }
        playerMeleeAttackPositionOffsetFromPlayer = transform.position - playerTransform.position;

        registry.sprites.insert(
            playerMeleeAttackEntity,
            {
                    dimensions,
                    20,
                    EFFECT_ASSET_ID::SPRITE,
                    TEXTURE_ASSET_ID::BOX
            }
        );

        if(Mix_PlayChannel(-1, world->sword_sound, 0) == -1) 
        {
            printf("Mix_PlayChannel: %s\n",Mix_GetError());
        }
    }
}

void PlayerSystem::PlayerAttackStep()
{
    if(registry.colliders.has(playerMeleeAttackEntity))
    {
        registry.colliders.remove(playerMeleeAttackEntity);
    }
}

void PlayerSystem::PausedStep(float deltaTime)
{
    bLeveledUpLastFrame = false;
}

void PlayerSystem::PrePhysicsStep(float deltaTime)
{
    if(registry.players.entities.empty()) { return; }
    playerEntity = registry.players.entities[0];
    playerComponentPtr = &registry.players.components[0];

    PlayerAttackPrePhysicsStep(deltaTime);
}

void PlayerSystem::Step(float deltaTime)
{
    if(registry.players.entities.empty()) { return; }

    MotionComponent& playerMotion = registry.motions.get(playerEntity);
    SpriteComponent& playerSprite = registry.sprites.get(playerEntity);
    TransformComponent& playerTransform = registry.transforms.get(playerEntity);
    HolderComponent& playerHolder = registry.holders.get(playerEntity);

    CheckIfLevelUp();

    PlayerAttackStep();

    HandleBasicMovementInput(playerMotion, *playerComponentPtr);
    HandleItemInteractionInput(playerHolder);
    ResolveComplexMovement(deltaTime, playerMotion);
    HandleSpriteSheetFrame(deltaTime, playerMotion, playerSprite);
}