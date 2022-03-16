// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include "player_system.hpp"
#include "input.hpp"
#include "levels.cpp"

WorldSystem::WorldSystem()
	: window(nullptr)
    , renderer(nullptr)
    , gameIsRunning(true)
    , currentGameMode(MODE_MAINMENU)
    , currentGameStage(GAME_NOT_STARTED)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

    allPossibleMutations.push_back({
        "Quick Feet",
        "Faster movement speed",
        SpriteComponent(),
        [](Entity mutatedEntity){
            if(registry.players.has(mutatedEntity))
            {
                Player& playerComponent = registry.players.get(mutatedEntity);
                playerComponent.movementSpeed += 16.f;
            }
        }
    });

    allPossibleMutations.push_back({
        "Heart of Steel",
        "Increased health",
        SpriteComponent(),
        [](Entity mutatedEntity){
            HealthBar& entityHealthComponent = registry.healthBar.get(mutatedEntity);
            entityHealthComponent.health += 30.f;
            entityHealthComponent.maxHealth += 30.f;
        }
    });

    allPossibleMutations.push_back({
        "Wolfsbane",
        "More powerful attacks",
        SpriteComponent(),
        [](Entity mutatedEntity){
            if(registry.players.has(mutatedEntity))
            {
                Player& playerComponent = registry.players.get(mutatedEntity);
                playerComponent.attackPower += 15;
            }
        }
    });

    allPossibleMutations.push_back({
        "Raging Bull",
        "Faster movement speed and increased attack power",
        SpriteComponent(),
        [](Entity mutatedEntity){
            if(registry.players.has(mutatedEntity))
            {
                Player& playerComponent = registry.players.get(mutatedEntity);
                playerComponent.movementSpeed += 10.f;
                playerComponent.attackPower += 10;
            }
        }
    });

    allPossibleMutations.push_back({
        "Oil of Vitriol",
        "Lose health in exchange for strength",
        SpriteComponent(),
        [](Entity mutatedEntity){
            HealthBar& entityHealthComponent = registry.healthBar.get(mutatedEntity);
            entityHealthComponent.maxHealth -= 50.f;
            if(entityHealthComponent.health > entityHealthComponent.maxHealth)
            {
                entityHealthComponent.health = entityHealthComponent.maxHealth;
            }
            if(registry.players.has(mutatedEntity))
            {
                Player& playerComponent = registry.players.get(mutatedEntity);
                playerComponent.movementSpeed += 7.f;
                playerComponent.attackPower += 10;
                playerComponent.meleeAttackCooldown *= 0.6f;
            }
        }
    });

    allPossibleMutations.push_back({
        "Shinobi's Blood",
        "Faster attack speed",
        SpriteComponent(),
        [](Entity mutatedEntity){
            if(registry.players.has(mutatedEntity))
            {
                Player& playerComponent = registry.players.get(mutatedEntity);
                playerComponent.movementSpeed += 5.f;
                playerComponent.meleeAttackCooldown *= 0.5f;
            }
        }
    });

    allPossibleMutations.push_back({
        "Windblade",
        "Wider attack arc",
        SpriteComponent(),
        [](Entity mutatedEntity){
            if(registry.players.has(mutatedEntity))
            {
                Player& playerComponent = registry.players.get(mutatedEntity);
                playerComponent.meleeAttackArc += 16;
            }
        }
    });

    allPossibleMutations.push_back({
        "Long Arms",
        "Increased attack range",
        SpriteComponent(),
        [](Entity mutatedEntity){
            if(registry.players.has(mutatedEntity))
            {
                Player& playerComponent = registry.players.get(mutatedEntity);
                playerComponent.meleeAttackRange += 14;
            }
        }
    });

    allPossibleMutations.push_back({
        "Warrior Spirit",
        "Larger attack arc and range",
        SpriteComponent(),
        [](Entity mutatedEntity){
            if(registry.players.has(mutatedEntity))
            {
                Player& playerComponent = registry.players.get(mutatedEntity);
                playerComponent.meleeAttackArc += 4;
                playerComponent.meleeAttackRange += 6;
            }
        }
    });

}

void WorldSystem::HandleMutations()
{
    for(size_t i = 0; i < registry.mutations.size(); ++i)
    {
        std::vector<Mutation>& activeMutations = registry.mutations.components[i].mutations;
        const Entity& mutatedEntity = registry.mutations.entities[i];
        for(Mutation& mutation : activeMutations)
        {
            if(mutation.bTriggered == false)
            {
                mutation.effect(mutatedEntity);
                mutation.bTriggered = true;
                if(Mix_PlayChannel(-1, gain_mutation_sound, 0) == -1) 
                {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
            }
        }
    }
}

void WorldSystem::init(RenderSystem* renderer_arg, PlayerSystem* player_sys_arg, AISystem* ai_sys_arg)
{
    this->renderer = renderer_arg;
    this->playerSystem = player_sys_arg;
    this->aiSystem = ai_sys_arg;

    loadAllContent();

    SetCurrentMode(MODE_MAINMENU);

	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");
}

void WorldSystem::cleanUp()
{
    unloadAllContent();
}

void WorldSystem::StartNewRun()
{
    printf("Starting new run.\n");

    // ENTER THE GAME MODE
    SetCurrentMode(MODE_INGAME);

    // Set the randomizer seed (deterministic - if two runs use the same seed, they will have exactly the same randomizations)
    srand((u32) time(nullptr));
    u32 seed = rand()%1000000000;
    SetRandomizerSeed(seed);

    StartNewStage(CHAPTER_ONE_STAGE_ONE);
}

void WorldSystem::StartNewStage(GAMELEVELENUM stage)
{
// SAVE PLAYER DATA
    auto playerPlayerComponent = Player();
    auto playerHealthComponent = HealthBar();
    auto playerActiveMutationsComponent = ActiveMutationsComponent();
    if(stage != CHAPTER_ONE_STAGE_ONE)
    {  
        playerPlayerComponent = registry.players.get(player);
        playerHealthComponent = registry.healthBar.get(player);
        playerActiveMutationsComponent = registry.mutations.get(player);
    }

// CLEAR STUFF FROM LAST STAGE
    // registry.list_all_components(); // Debugging for memory/component leaks
    // Remove all entities that we created
    while (registry.transforms.entities.size() > 0)
        registry.remove_all_components_of(registry.transforms.entities.back());
    // registry.list_all_components(); // Debugging for memory/component leaks

// CHECK IF GAME SHOULD END
    if(stage == END_THE_GAME)
    {
        printf("Ending run.\n");
        SetCurrentMode(MODE_MAINMENU);
        return;
    }

// GENERATE NEW STAGE
    printf("Starting new stage.\n");
    currentGameStage = stage;

    // Create random level
    GenerateNewLevel(stage);
    aiSystem->Init(levelForAI());
    renderer->cameraBoundMin = currentLevelData.cameraBoundMin;
    renderer->cameraBoundMax = currentLevelData.cameraBoundMax;
    SpawnLevelEntities();

// LOAD PLAYER DATA
    if(stage != CHAPTER_ONE_STAGE_ONE)
    {  
        registry.players.get(player) = playerPlayerComponent;
        registry.healthBar.get(player) = playerHealthComponent;
        registry.mutations.get(player) = playerActiveMutationsComponent;
    }
}

void WorldSystem::SpawnLevelEntities()
{
    // Create player
    player = createPlayer(currentLevelData.playerStart);

    createSword(currentLevelData.playerStart);

    createBow(currentLevelData.playerStart + vec2({10.f, 0}));

    // Create enemies
    for(vec2 groundEnemySpawn : currentLevelData.groundMonsterSpawns)
    {
        CreateGoblinEnemy(groundEnemySpawn);
    }
    for(vec2 flyingEnemySpawn : currentLevelData.flyingMonsterSpawns)
    {
        CreateBatEnemy(flyingEnemySpawn);
    }

    // Create shop items
    for(vec2 shopItemSpawn : currentLevelData.shopItemSpawns)
    {
        CreateShopItem(shopItemSpawn, rand()%5);
    }
}

void WorldSystem::loadAllContent()
{
    background_music = Mix_LoadMUS(audio_path("hadesmusiclmao.wav").c_str());
    chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
    chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());
    sword_sound = Mix_LoadWAV(audio_path("sword_sound.wav").c_str());
    monster_hurt_sound = Mix_LoadWAV(audio_path("monster_hurt.wav").c_str());
    monster_death_sound = Mix_LoadWAV(audio_path("monster_death.wav").c_str());
    player_hurt_sound = Mix_LoadWAV(audio_path("player_hurt.wav").c_str());
    player_death_sound = Mix_LoadWAV(audio_path("death_effect.wav").c_str());
    gain_mutation_sound = Mix_LoadWAV(audio_path("mutation.wav").c_str());
    player_levelup_sound = Mix_LoadWAV(audio_path("levelup.wav").c_str());
    blip_select_sound = Mix_LoadWAV(audio_path("blip_select.wav").c_str());

    if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr
        || sword_sound == nullptr
        || monster_hurt_sound == nullptr
        || monster_death_sound == nullptr
        || player_hurt_sound == nullptr
        || player_death_sound == nullptr
        || gain_mutation_sound == nullptr
        || player_levelup_sound == nullptr
        || blip_select_sound == nullptr) {
        fprintf(stderr, "Failed to load sounds. Make sure the audio directory is present.");
    }

    LoadAllLevelData();
}

void WorldSystem::unloadAllContent()
{
    // Destroy music components
    if (background_music != nullptr)
        Mix_FreeMusic(background_music);
    if (chicken_dead_sound != nullptr)
        Mix_FreeChunk(chicken_dead_sound);
    if (chicken_eat_sound != nullptr)
        Mix_FreeChunk(chicken_eat_sound);
    if (sword_sound != nullptr)
        Mix_FreeChunk(sword_sound);
    if (monster_hurt_sound != nullptr)
        Mix_FreeChunk(monster_hurt_sound);
    if (monster_death_sound != nullptr)
        Mix_FreeChunk(monster_death_sound);
    if (player_hurt_sound != nullptr)
        Mix_FreeChunk(player_hurt_sound);
    if (player_death_sound != nullptr)
        Mix_FreeChunk(player_death_sound);
    if (gain_mutation_sound != nullptr)
        Mix_FreeChunk(gain_mutation_sound);
    if (player_levelup_sound != nullptr)
        Mix_FreeChunk(player_levelup_sound);
    if (blip_select_sound != nullptr)
        Mix_FreeChunk(blip_select_sound);
    Mix_CloseAudio();

    // Destroy all created components
    registry.clear_all_components();
}

std::vector<std::vector<int>> WorldSystem::levelForAI() {
    std::vector<std::vector<int>> newLevelTiles;
    for (int i = 0; i < NUMTILESWIDE; i++) {
        std::vector<int> currentLayer;
        for (int j = 0; j < NUMTILESTALL; j++) {
            currentLayer.push_back((levelTiles[i][j] != 0) ? 1 : 0);
        }
        newLevelTiles.push_back(currentLayer);
    }
    return newLevelTiles;
}

void WorldSystem::SetCurrentMode(GAMEMODE mode)
{
    currentGameMode = mode;

    switch(mode)
    {
        case MODE_MAINMENU:{
            renderer->bgTexId = { TEXTURE_ASSET_ID::MAINMENUBG };
        }break;
        case MODE_INGAME:{
            renderer->bgTexId = {
                TEXTURE_ASSET_ID::BG_LAYER1,
                TEXTURE_ASSET_ID::BG_LAYER2,
                TEXTURE_ASSET_ID::BG_LAYER3,
                TEXTURE_ASSET_ID::BG_LAYER4,
                TEXTURE_ASSET_ID::BG_LAYER5
            };
        }break;
    }
}

void WorldSystem::UpdateMode()
{
    if(GetCurrentMode() == MODE_MAINMENU)
    {
        if(Input::HasKeyBeenPressed(SDL_SCANCODE_RETURN))
        {
            StartNewRun();
        }
    }   
}

// Update our game world
bool WorldSystem::step(float deltaTime) {

	// Remove debug info from the last Step
	while (registry.debugComponents.entities.size() > 0)
        registry.remove_all_components_of(registry.debugComponents.entities.back());

    // Check if player dead and if run should end
    if(registry.players.size() > 0)
    {      
        Player playerComponent = registry.players.components[0];
        if(playerComponent.bDead)
        {
            darkenGameFrame = false;
            playerComponent.bDead = false;
            StartNewStage(END_THE_GAME);
        }
    }

    if(Input::HasKeyBeenPressed(SDL_SCANCODE_U))
    {
        registry.mutations.get(registry.players.entities[0]).mutations.push_back(allPossibleMutations[1]);
    }

    auto& playerProjectileRegistry = registry.playerProjectiles;
    for(int i = 0; i < playerProjectileRegistry.size(); i++)
    {
        playerProjectileRegistry.components[i].elapsed_time += deltaTime;

        if (playerProjectileRegistry.components[i].elapsed_time > 5) {
            registry.remove_all_components_of(playerProjectileRegistry.entities[i]);
        }
    }

//  float min_counter_ms = 3000.f;
//	for (Entity entity : registry.deathTimers.entities) {
//		// progress timer
//		DeathTimer& counter = registry.deathTimers.get(entity);
//		counter.counter_ms -= elapsed_ms_since_last_update;
//		if(counter.counter_ms < min_counter_ms){
//		    min_counter_ms = counter.counter_ms;
//		}
//
//		// restart the game once the death timer expired
//		if (counter.counter_ms < 0) {
//			registry.deathTimers.remove(entity);
//            restart_game();
//			return true;
//		}
//	}

	return true;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() 
{
    Player& playerComponent = registry.players.get(player);
    TransformComponent& playerTransform = registry.transforms.get(player);
    MotionComponent& playerMotion = registry.motions.get(player);
    CollisionComponent& playerCollider = registry.colliders.get(player);
    HealthBar& playerHealth = registry.healthBar.get(player);

	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisionEvents;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {

        const CollisionEvent colEvent = collisionsRegistry.components[i];
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = colEvent.other;

        if (registry.enemy.has(entity))
        {
            bool is_thrown_weapon = registry.items.has(entity_other)
                    && registry.items.get(entity_other).thrown
                    && (!registry.items.get(entity_other).grounded || abs(registry.motions.get(entity_other).velocity.x) > 0);

            if(entity_other.GetTag() == TAG_PLAYERMELEEATTACK || is_thrown_weapon)
            {
                HealthBar& enemyHealth = registry.healthBar.get(entity);
                enemyHealth.TakeDamage((float) playerComponent.attackPower, (float) playerComponent.attackVariance);

                if (entity_other.GetTag() == TAG_PLAYERMELEEATTACK)
                {
                    // Move the player a little bit - its more fun
                    if(playerSystem->lastAttackDirection == 3)
                    {
                        auto& playerMotion = registry.motions.get(player);
                        playerMotion.velocity.y = std::min(-playerMotion.velocity.y, -180.f);
                        playerMotion.velocity.x *= 1.7f;
                    }
                    else if(playerSystem->lastAttackDirection == 0 || playerSystem->lastAttackDirection == 1)
                    {
                        auto& playerMotion = registry.motions.get(player);
                        float bumpXVel = std::max(std::abs(playerMotion.velocity.x) * 1.5f, 150.f);
                        playerMotion.velocity.x = playerSystem->lastAttackDirection == 0 ? bumpXVel : -bumpXVel;
                    }
                }

                if(enemyHealth.health <= 0.f && !registry.deathTimers.has(entity)) // TODO: Experience and/or money as drops to be picked up
                {
                    registry.deathTimers.emplace(entity);
                    registry.colliders.remove(entity);
                    registry.collisionEvents.remove(entity);
                    MotionComponent& motion = registry.motions.get(entity);
                    motion.acceleration = { 0.f, 0.f };
                    motion.velocity = { 0.f, 0.f };

                    playerComponent.experience += 40.f;
                    if(Mix_PlayChannel(-1, monster_death_sound, 0) == -1) 
                    {
                        printf("Mix_PlayChannel: %s\n",Mix_GetError());
                    }
                }
                else
                {
                    if(Mix_PlayChannel(-1, monster_hurt_sound, 0) == -1) 
                    {
                        printf("Mix_PlayChannel: %s\n",Mix_GetError());
                    }
                }

                *GlobalPauseForSeconds = 0.1f;
            }
        }

		if (registry.players.has(entity)) 
        {
            CheckCollisionWithBlockable(entity, entity_other);

            if (entity_other.GetTag() == TAG_SPIKE)
            {
                if (playerMotion.velocity.y > 0.f) // only hurt when falling on spikes
                {
                    playerHealth.health = -9999.f;
                    if(Mix_PlayChannel(-1, player_hurt_sound, 0) == -1) 
                    {
                        printf("Mix_PlayChannel: %s\n",Mix_GetError());
                    }
                }
            }

			if (registry.enemy.has(entity_other)) 
            {
                Enemy& enemy = registry.enemy.get(entity_other);
				if (enemy.playerHurtCooldown <= 0.f && playerHealth.health > 0) 
                {
                    enemy.playerHurtCooldown = 2.f;
                    playerHealth.TakeDamage(8, 5);
                    if(Mix_PlayChannel(-1, player_hurt_sound, 0) == -1) 
                    {
                        printf("Mix_PlayChannel: %s\n",Mix_GetError());
                    }
				}
			}

            if (registry.enemyprojectile.has(entity_other))
            {
                if (playerHealth.health > 0)
                {
                    playerHealth.TakeDamage(10, 5);
                    if(Mix_PlayChannel(-1, player_hurt_sound, 0) == -1) 
                    {
                        printf("Mix_PlayChannel: %s\n",Mix_GetError());
                    }
                }

                auto& enemyprojectileRegistry = registry.enemyprojectile;
                Entity fire_entity = enemyprojectileRegistry.entities[0];
                registry.remove_all_components_of(fire_entity);
             }

            if (entity_other.GetTag() == TAG_LEVELENDPOINT && Input::GameUpHasBeenPressed())
            {
                StartNewStage((GAMELEVELENUM) ((u8) currentGameStage + 1));
            }

//			// Checking Player - Deadly collisions
//			if (registry.deadlys.has(entity_other)) {
//
//			}
//			// Checking Player - Eatable collisions
//			else if (registry.eatables.has(entity_other)) {
//
//			}
		}

        if (registry.enemy.has(entity)) {
            CheckCollisionWithBlockable(entity, entity_other);
        }

        if(registry.holders.has(entity))
        {
            HolderComponent& holder = registry.holders.get(entity);

            if (registry.items.has(entity_other) && !registry.playerProjectiles.has(entity_other))
            {
                holder.near_weapon = entity_other;
            }
            else
            {
                // Assign to fake entity
                holder.near_weapon = Entity();
            }
        }

        if(registry.items.has(entity))
        {
            if(registry.items.get(entity).collidableWithEnvironment)
            {
                CheckCollisionWithBlockable(entity, entity_other, true, true);

                MotionComponent& itemMotion = registry.motions.get(entity);
                float deceleration = 3.f;
                deceleration = min(deceleration, abs(itemMotion.velocity.x));

                if (itemMotion.velocity.x > 0) {
                    itemMotion.velocity.x -= deceleration;
                } else if (itemMotion.velocity.x < 0) {
                    itemMotion.velocity.x += deceleration;
                }
            }
        }
	}
	// Remove all collisions from this simulation Step
	registry.collisionEvents.clear();


    if(playerHealth.health <= 0.f && !playerComponent.bDead)
    {
        // DEAD
        if(Mix_PlayChannel(-1, player_death_sound, 0) == -1) 
        {
            printf("Mix_PlayChannel: %s\n",Mix_GetError());
        }
        playerComponent.bDead = true;
        darkenGameFrame = true;
        *GlobalPauseForSeconds = 3.f;
    }
}

void WorldSystem::CheckCollisionWithBlockable(Entity entity_resolver, Entity entity_other, bool bounce_x, bool is_item)
{
    if (entity_other.GetTag() == TAG_PLAYERBLOCKABLE)
    {
        if (registry.colliders.has(entity_resolver) && registry.colliders.has(entity_other))
        {
            CollisionComponent& resolverCollider = registry.colliders.get(entity_resolver);
            CollisionComponent& otherCollider = registry.colliders.get(entity_other);

            /** Note(Kevin): This collisionCheckAgain is required because as we resolve collisions
             *  by moving entities around, the initial collection of collision events may become outdated.
             *  Checking that the two entities are still colliding is not a perfect solution (if there
             *  even is one), but it should be good enough... We can revisit this and attempt other
             *  solutions down the line if needed. */
            CollisionInfo collisionCheckAgain = CheckCollision(resolverCollider, otherCollider);
            if (collisionCheckAgain.collides)
            {
                TransformComponent& resolverTransform = registry.transforms.get(entity_resolver);
                if (abs(collisionCheckAgain.collision_overlap.x) < abs(collisionCheckAgain.collision_overlap.y))
                {
                    MotionComponent& resolverMotion = registry.motions.get(entity_resolver);
                    resolverTransform.position.x += collisionCheckAgain.collision_overlap.x;
                    resolverCollider.collider_position.x += collisionCheckAgain.collision_overlap.x;

                    if (bounce_x)
                    {
                        resolverMotion.velocity.x = 0.25f * -resolverMotion.velocity.x;
                    } else
                    {
                        resolverMotion.velocity.x = 0.f;
                    }
                }
                else
                {
                    resolverTransform.position.y += collisionCheckAgain.collision_overlap.y;
                    resolverCollider.collider_position.y += collisionCheckAgain.collision_overlap.y;

                    if (is_item && collisionCheckAgain.collision_overlap.y > 0)
                    {
                        registry.items.get(entity_resolver).grounded = true;
                    }
                }
            }
        }
    }
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return !gameIsRunning;
}

void WorldSystem::SDLProcessEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_KEYDOWN:{
                Input::ProcessSDLKeyDownEvent(event.key);
            }break;
            case SDL_KEYUP:{
                Input::ProcessSDLKeyUpEvent(event.key);
            }break;

            case SDL_CONTROLLERBUTTONDOWN:{
                Input::ProcessSDLControllerButtonDownEvent(event.cbutton);
            }break;
            case SDL_CONTROLLERBUTTONUP:{
                Input::ProcessSDLControllerButtonUpEvent(event.cbutton);
            }break;
            case SDL_CONTROLLERAXISMOTION:{
                Input::ProcessSDLControllerAxisEvent(event.caxis);
            }break;
            case SDL_CONTROLLERDEVICEADDED:{
                Input::SDLControllerConnected(event.cdevice.which);
            }break;
            case SDL_CONTROLLERDEVICEREMOVED:{
                Input::SDLControllerRemoved(event.cdevice.which);
            }break;

            case SDL_WINDOWEVENT:{
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:{
                        renderer->UpdateBackBufferSize();
                    }break;
                }
            }break;

            case SDL_QUIT:{
                gameIsRunning = false;
            }break;
        }
    }
}
