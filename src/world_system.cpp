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

    Mutation fastFeet = Mutation{"fastFeet", 5, 0, 0, SpriteComponent()};
    Mutation powerfulHands = Mutation{"powerfulHands", 0, 5, 0, SpriteComponent()};
    Mutation heartOfSteel = Mutation{"heartOfSteel", 0, 0, 5, SpriteComponent()};
    Mutation bullPower = Mutation{"rage", 3, 3, 0, SpriteComponent()};
    Mutation invisibleShield = Mutation{"ironDefence", 3, 0, 3, SpriteComponent()};
    Mutation allPossibleMutations [5] = {fastFeet, powerfulHands, heartOfSteel, bullPower, invisibleShield};

}

void WorldSystem::init(RenderSystem* renderer_arg, PlayerSystem* player_sys_arg, AISystem* ai_sys_arg)
{
    this->renderer = renderer_arg;
    this->playerSystem = player_sys_arg;
    this->aiSystem = ai_sys_arg;

    loadAllContent();

    SetCurrentMode(MODE_MAINMENU);

	// Playing background music indefinitely
	//Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");
}

void WorldSystem::cleanUp()
{
    unloadAllContent();
}

void WorldSystem::StartNewRun()
{
    printf("Starting new run.\n");

    // REINSTANTIATE SOME THINGS
    *playerSystem = PlayerSystem();

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

// CLEAR STUFF FROM LAST STAGE
    // registry.list_all_components(); // Debugging for memory/component leaks
    // Remove all entities that we created
    while (registry.transforms.entities.size() > 0)
        registry.remove_all_components_of(registry.transforms.entities.back());
    // registry.list_all_components(); // Debugging for memory/component leaks
    player = Entity();

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
}

void WorldSystem::SpawnLevelEntities()
{
    // Create player
    player = createPlayer(currentLevelData.playerStart);

    createSword(currentLevelData.playerStart);

    // Create enemies
    for(vec2 enemySpawn : currentLevelData.monsterSpawns)
    {
        createEnemy(enemySpawn);
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
    monster_hit_sound = Mix_LoadWAV(audio_path("sword_sound.wav").c_str());
    player_hurt_sound = Mix_LoadWAV(audio_path("player_hurt.wav").c_str());
    player_death_sound = Mix_LoadWAV(audio_path("death_effect.wav").c_str());

    if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr
        || sword_sound == nullptr
        || monster_hit_sound == nullptr
        || player_hurt_sound == nullptr
        || player_death_sound == nullptr) {
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
    if (monster_hit_sound != nullptr)
        Mix_FreeChunk(monster_hit_sound);
    if (player_hurt_sound != nullptr)
        Mix_FreeChunk(player_hurt_sound);
    if (player_death_sound != nullptr)
        Mix_FreeChunk(player_death_sound);
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
            if(entity_other.GetTag() == TAG_PLAYERMELEEATTACK)
            {
                if(Mix_PlayChannel(-1, monster_hit_sound, 0) == -1) 
                {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }

                HealthBar& enemyHealth = registry.healthBar.get(entity);
                enemyHealth.health -= playerComponent.attackPower;

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

                if(enemyHealth.health <= 0.f) // TODO: Remove from here and probably move to ai systems
                {
                    registry.remove_all_components_of(entity);
                    playerComponent.experience += 40.f;
                }

                *GlobalPauseForSeconds = 0.1f;
            }
        }

		if (registry.players.has(entity)) 
        {
            CheckCollisionWithBlockable(entity, entity_other);

            if (entity_other.GetTag() == TAG_SPIKE)
            {
                if (playerHealth.health > 0)
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
                    playerHealth.health -= 10;
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
                    playerHealth.health -= 15;
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

            if (registry.items.has(entity_other))
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
                CheckCollisionWithBlockable(entity, entity_other);
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

void WorldSystem::CheckCollisionWithBlockable(Entity entity_resolver, Entity entity_other)
{
    if (entity_other.GetTag() == TAG_PLAYERBLOCKABLE)
    {
        CollisionComponent& resolverCollider = registry.colliders.get(entity_resolver);
        CollisionComponent& otherCollider = registry.colliders.get(entity_other);

        /** Note(Kevin): This collisionCheckAgain is required because as we resolve collisions
         *  by moving entities around, the initial collection of collision events may become outdated.
         *  Checking that the two entities are still colliding is not a perfect solution (if there
         *  even is one), but it should be good enough... We can revisit this and attempt other
         *  solutions down the line if needed. */
        CollisionInfo collisionCheckAgain = CheckCollision(resolverCollider, otherCollider);
        if(collisionCheckAgain.collides)
        {
            TransformComponent& resolverTransform = registry.transforms.get(entity_resolver);
            if(abs(collisionCheckAgain.collision_overlap.x) < abs(collisionCheckAgain.collision_overlap.y))
            {
                MotionComponent& resolverMotion = registry.motions.get(entity_resolver);
                resolverTransform.position.x += collisionCheckAgain.collision_overlap.x;
                resolverCollider.collider_position.x += collisionCheckAgain.collision_overlap.x;
                resolverMotion.velocity.x = 0.f;
            }
            else
            {
                resolverTransform.position.y += collisionCheckAgain.collision_overlap.y;
                resolverCollider.collider_position.y += collisionCheckAgain.collision_overlap.y;
            }
        }
    }
}

void WorldSystem::handle_mutations(Mutation currentMutation) {
    auto& mutationRegistry = registry.mutationComponent;
    for (uint i = 0; i < mutationRegistry.components.size(); i++) {
        const MutationComponent mutComp = mutationRegistry.components[i];
        Entity entity = mutationRegistry.entities[i];

        if (registry.players.has(entity)) {
            Player& player = registry.players.get(entity);
            TransformComponent& playerTransform = registry.transforms.get(entity);
            MotionComponent& playerMotion = registry.motions.get(entity);
            CollisionComponent& playerCollider = registry.colliders.get(entity);
            HealthBar& playerHealth = registry.healthBar.get(entity);

            playerHealth.health += currentMutation.healthEffect;
            player.attackPower += currentMutation.attackPowerEffect;

            printf("Mutation selected");
            printf("Changed player health by: %d. Current player health is: %f \n", currentMutation.healthEffect, playerHealth.health);
            printf("Changed player attackPower by: %d. Current player attackPower is: %d \n", currentMutation.attackPowerEffect, player.attackPower);

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
