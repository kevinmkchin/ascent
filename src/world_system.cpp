// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include "player_system.hpp"
#include "ui_system.hpp"
#include "input.hpp"
#include "levels.cpp"
#include "console.hpp"


WorldSystem::WorldSystem()
        : window(nullptr), renderer(nullptr), gameIsRunning(true), currentGameMode(MODE_MAINMENU),
          currentGameStage(GAME_NOT_STARTED) {
    // Seeding rng with random device
    rng = std::default_random_engine(std::random_device()());

    allPossibleMutations.push_back({
                                           "Quick Feet",
                                           "Faster movement speed",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
                                                   playerComponent.movementSpeed += 16.f;
                                               }
                                           }
                                   });

    allPossibleMutations.push_back({
                                           "Heart of Steel",
                                           "Increased health",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               HealthBar &entityHealthComponent = registry.healthBar.get(mutatedEntity);
                                               entityHealthComponent.health += 30.f;
                                               entityHealthComponent.maxHealth += 30.f;
                                           }
                                   });

    allPossibleMutations.push_back({
                                           "Wolfsbane",
                                           "More powerful attacks",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
                                                   playerComponent.attackPower += 15;
                                               }
                                           }
                                   });

    allPossibleMutations.push_back({
                                           "Raging Bull",
                                           "Faster movement speed and increased attack power",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
                                                   playerComponent.movementSpeed += 10.f;
                                                   playerComponent.attackPower += 10;
                                               }
                                           }
                                   });

    allPossibleMutations.push_back({
                                           "Oil of Vitriol",
                                           "Lose health in exchange for strength",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               HealthBar &entityHealthComponent = registry.healthBar.get(mutatedEntity);
                                               entityHealthComponent.maxHealth -= 50.f;
                                               if (entityHealthComponent.health > entityHealthComponent.maxHealth) {
                                                   entityHealthComponent.health = entityHealthComponent.maxHealth;
                                               }
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
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
                                           [](Entity mutatedEntity) {
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
                                                   playerComponent.movementSpeed += 5.f;
                                                   playerComponent.meleeAttackCooldown *= 0.5f;
                                               }
                                           }
                                   });

    allPossibleMutations.push_back({
                                           "Windblade",
                                           "Wider attack arc",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
                                                   playerComponent.meleeAttackArc += 16;
                                               }
                                           }
                                   });

    allPossibleMutations.push_back({
                                           "Long Arms",
                                           "Increased attack range",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
                                                   playerComponent.meleeAttackRange += 14;
                                               }
                                           }
                                   });

    allPossibleMutations.push_back({
                                           "Warrior Spirit",
                                           "Larger attack arc and range",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
                                                   playerComponent.meleeAttackArc += 4;
                                                   playerComponent.meleeAttackRange += 6;
                                               }
                                           }
                                   });

    allPossibleMutations.push_back({
                                           "Moonspirit Wings",
                                           "Gain an extra jump",
                                           SpriteComponent(),
                                           [](Entity mutatedEntity) {
                                               if (registry.players.has(mutatedEntity)) {
                                                   Player &playerComponent = registry.players.get(mutatedEntity);
                                                   playerComponent.maxJumps++;
                                               }
                                           }
                                   });

    get_console().bind_cmd("die", 
        [this](std::istream& is, std::ostream& os){
            auto& playerHealth = registry.healthBar.get(this->player);
            playerHealth.health = -99.f;
        });

    get_console().bind_cmd("restart", 
        [this](std::istream& is, std::ostream& os){
            if(this->currentGameMode == MODE_INGAME)
            {
                this->StartNewStage(END_THE_GAME);
                this->uiSystem->cachedGameStage = GAME_NOT_STARTED;
                this->StartNewRun();
            }
            else
            {
                console_printf("'restart' command only works while in game...\n");
            }
        });

    get_console().bind_cmd("next_stage",
        [this](std::istream& is, std::ostream& os){
            if(this->currentGameMode == MODE_INGAME)
            {
                StartNewStage((GAMELEVELENUM) ((u8) this->currentGameStage + 1));
            }
            else
            {
                console_printf("'next_stage' command only works while in game...\n");
            }
        });

    get_console().bind_cmd("gotoshop",
        [this](std::istream& is, std::ostream& os){
            if(this->currentGameMode == MODE_INGAME)
            {
                if(currentLevelData.shopItemSpawns.empty())
                {
                    console_printf("There is no shop...\n");
                }
                else
                {
                    auto& playerTransform = registry.transforms.get(this->player);
                    playerTransform.position = currentLevelData.shopItemSpawns[0];
                }
            }
            else
            {
                console_printf("'gotoshop' command only works while in game...\n");
            }
        });

    get_console().bind_cmd("godmode",
        [this](std::istream& is, std::ostream& os){
            if(this->currentGameMode == MODE_INGAME)
            {
                auto& playerHP = registry.healthBar.get(this->player);
                playerHP.bInvincible = !playerHP.bInvincible;
                if(playerHP.bInvincible) {
                    console_printf("godmode ON\n");
                } else {
                    console_printf("godmode OFF\n");
                }
            }
            else
            {
                console_printf("'next_stage' command only works while in game...\n");   
            }
        });
}

void WorldSystem::HandleMutations() {
    for (size_t i = 0; i < registry.mutations.size(); ++i) {
        std::vector<Mutation> &activeMutations = registry.mutations.components[i].mutations;
        const Entity &mutatedEntity = registry.mutations.entities[i];
        for (Mutation &mutation: activeMutations) {
            if (mutation.bTriggered == false) {
                mutation.effect(mutatedEntity);
                mutation.bTriggered = true;
                if (Mix_PlayChannel(-1, gain_mutation_sound, 0) == -1) {
                    printf("Mix_PlayChannel: %s\n", Mix_GetError());
                }
            }
        }
    }
}

void WorldSystem::init(RenderSystem *renderer_arg, PlayerSystem *player_sys_arg, AISystem *ai_sys_arg) {
    this->renderer = renderer_arg;
    this->playerSystem = player_sys_arg;
    this->aiSystem = ai_sys_arg;

    loadAllContent();

    SetCurrentMode(MODE_MAINMENU);

    // Playing background music indefinitely
    // TODO(Kevin): uncomment  // Mix_PlayMusic(background_music, -1);
    fprintf(stderr, "Loaded music\n");
}

void WorldSystem::cleanUp() {
    unloadAllContent();
}

void WorldSystem::StartNewRun() {
    printf("Starting new run.\n");

    // ENTER THE GAME MODE
    SetCurrentMode(MODE_INGAME);

    // Set the randomizer seed (deterministic - if two runs use the same seed, they will have exactly the same randomizations)
    srand((u32) time(nullptr));
    u32 seed = rand() % 1000000000;
    SetRandomizerSeed(seed);

    StartNewStage(CHAPTER_TUTORIAL);
}

void WorldSystem::HelpMode() {

    SetCurrentMode(MODE_HELP);

    if (Input::HasKeyBeenPressed(SDL_SCANCODE_RETURN)) {
        SetCurrentMode(MODE_INGAME);
    }

}

void WorldSystem::StartNewStage(GAMELEVELENUM stage) {
// SAVE PLAYER DATA
    auto playerPlayerComponent = Player();
    auto playerHealthComponent = HealthBar();
    auto playerGoldComponent = GoldBar();
    auto playerActiveMutationsComponent = ActiveMutationsComponent();
    if (stage > CHAPTER_ONE_STAGE_ONE) {
        playerPlayerComponent = registry.players.get(player);
        playerHealthComponent = registry.healthBar.get(player);
        playerGoldComponent = registry.goldBar.get(player);
        playerActiveMutationsComponent = registry.mutations.get(player);
    }

// CLEAR STUFF FROM LAST STAGE
    // registry.list_all_components(); // Debugging for memory/component leaks
    // Remove all entities that we created
    while (registry.transforms.entities.size() > 0)
        registry.remove_all_components_of(registry.transforms.entities.back());
    while (registry.proximityTexts.entities.size() > 0)
        registry.remove_all_components_of(registry.proximityTexts.entities.back());
    // registry.list_all_components(); // Debugging for memory/component leaks

// CHECK IF GAME SHOULD END
    if (stage == END_THE_GAME) {
        printf("Ending run.\n");
        currentGameStage = GAME_NOT_STARTED;
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
    if (stage > CHAPTER_ONE_STAGE_ONE) {
        registry.players.get(player) = playerPlayerComponent;
        registry.healthBar.get(player) = playerHealthComponent;
        registry.goldBar.get(player) = playerGoldComponent;
        registry.mutations.get(player) = playerActiveMutationsComponent;
    }
}

void WorldSystem::SpawnLevelEntities() {
    // Create player
    player = createPlayer(currentLevelData.playerStart);
    CreateHelpSign(currentLevelData.playerStart, 16.f, vec2(0.f, -8.f), 8, "Make your way to the top of the mountain!");

    if (currentDifficulty == DIFFICULTY_EASY) {
        registry.healthBar.get(player).maxHealth += 50;
        registry.healthBar.get(player).health += 50;
        registry.players.get(player).attackPower += 10;
    }


    createSword(currentLevelData.playerStart);
    createBow(currentLevelData.playerStart + vec2({10.f, 0}));


    // Create enemies
    for (vec2 groundEnemySpawn : currentLevelData.groundMonsterSpawns) {

        int spawnType = rand() % 2;
        
        if (currentGameStage == CHAPTER_ONE_STAGE_ONE) {
            if (spawnType == 0) {
                CreateGoblinEnemy(groundEnemySpawn);
            }
            else {
                CreateSlimeEnemy(groundEnemySpawn);
            }
        }
        else if (currentGameStage == CHAPTER_TWO_STAGE_ONE) {
            if (spawnType == 0) {
                CreateMushroomEnemy(groundEnemySpawn);
            }
            else {
                CreateWormEnemy(groundEnemySpawn);
            }
        }
        else if (currentGameStage == CHAPTER_THREE_STAGE_ONE) {
            if (spawnType == 0) {
                CreateGoblinBomberEnemy(groundEnemySpawn);
            }
            else {
                CreateMushroomEnemy(groundEnemySpawn);
            }
        }
        else if (currentGameStage == CHAPTER_TUTORIAL) {
            CreateGoblinEnemy(groundEnemySpawn);
        }
    }
    for (vec2 flyingEnemySpawn: currentLevelData.flyingMonsterSpawns) {
        CreateBatEnemy(flyingEnemySpawn);
    }

    // Create shop items
    for (vec2 shopItemSpawn: currentLevelData.shopItemSpawns) {
        CreateShopItem(shopItemSpawn, rand() % 5);
    }
}

void WorldSystem::loadAllContent() {
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
    player_jump_on_enemy_sound = Mix_LoadWAV(audio_path("player_jump_on_enemy_sound.wav").c_str());
    coins_pickup_sound = Mix_LoadWAV(audio_path("coins_pickup.wav").c_str());
    points_pickup_sound = Mix_LoadWAV(audio_path("points_pickup.wav").c_str());

    if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr
        || sword_sound == nullptr
        || monster_hurt_sound == nullptr
        || monster_death_sound == nullptr
        || player_hurt_sound == nullptr
        || player_death_sound == nullptr
        || gain_mutation_sound == nullptr
        || player_levelup_sound == nullptr
        || blip_select_sound == nullptr
        || player_jump_on_enemy_sound == nullptr
        || coins_pickup_sound == nullptr
        || points_pickup_sound == nullptr) {
        fprintf(stderr, "Failed to load sounds. Make sure the audio directory is present.");
    }

    LoadAllLevelData();
}

void WorldSystem::unloadAllContent() {
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
    if (player_jump_on_enemy_sound != nullptr)
        Mix_FreeChunk(player_jump_on_enemy_sound);
    if (coins_pickup_sound != nullptr)
        Mix_FreeChunk(coins_pickup_sound);
    if (points_pickup_sound != nullptr)
        Mix_FreeChunk(points_pickup_sound);
    Mix_CloseAudio();

    // Destroy all created components
    registry.clear_all_components();
}

std::vector<std::vector<int>> WorldSystem::levelForAI() {
    std::vector<std::vector<int>> newLevelTiles;
    for (int i = 0; i < levelTiles.size(); i++) {
        std::vector<int> currentLayer;
        for (int j = 0; j < levelTiles[0].size(); j++) {
            currentLayer.push_back((levelTiles[i][j] != 0) ? 1 : 0);
        }
        newLevelTiles.push_back(currentLayer);
    }
    return newLevelTiles;
}

void WorldSystem::SetCurrentMode(GAMEMODE mode) {
    currentGameMode = mode;

    switch (mode) {
        case MODE_MAINMENU: {
            renderer->bgTexId = {
                    TEXTURE_ASSET_ID::BG_MENU_LAYER1,
                    TEXTURE_ASSET_ID::BG_MENU_LAYER2,
                    TEXTURE_ASSET_ID::BG_MENU_LAYER3,
                    TEXTURE_ASSET_ID::BG_MENU_LAYER4,
                    TEXTURE_ASSET_ID::BG_MENU_LAYER5
            };
        }
            break;
        case MODE_INGAME: {
            renderer->bgTexId = {
                    TEXTURE_ASSET_ID::BG_LAYER1,
                    TEXTURE_ASSET_ID::BG_LAYER2,
                    TEXTURE_ASSET_ID::BG_LAYER3,
                    TEXTURE_ASSET_ID::BG_LAYER4,
                    TEXTURE_ASSET_ID::BG_LAYER5
            };
        }
            break;
        case MODE_HELP: {
            renderer->bgTexId = {
                    TEXTURE_ASSET_ID::HELP_MENU,
            };
        }
            break;
    }
}

void WorldSystem::UpdateMode() {
    if (GetCurrentMode() == MODE_MAINMENU ) {
        if (Input::HasKeyBeenPressed(SDL_SCANCODE_RETURN)) {
            StartNewRun();
        }

        else if (Input::HasKeyBeenPressed(SDL_SCANCODE_H)) {
            HelpMode();
        }

    }

    else if (GetCurrentMode() == MODE_HELP) {
        if (Input::HasKeyBeenPressed(SDL_SCANCODE_RETURN)) {
            StartNewRun();
        }
        else if (Input::HasKeyBeenPressed(SDL_SCANCODE_M)) {
            SetCurrentMode(MODE_MAINMENU);
        }
    }
}

// Update our game world
bool WorldSystem::step(float deltaTime) {

    // Remove debug info from the last Step
    while (registry.debugComponents.entities.size() > 0)
        registry.remove_all_components_of(registry.debugComponents.entities.back());

    // Check if player dead and if run should end
    if (registry.players.size() > 0) {
        Player playerComponent = registry.players.components[0];
        if (playerComponent.bDead) {
            darkenGameFrame = false;
            playerComponent.bDead = false;
            StartNewStage(END_THE_GAME);
        }
    }

    auto& playerProjectileRegistry = registry.playerProjectiles;
    for(int i = 0; i < playerProjectileRegistry.size(); i++)
    {
        playerProjectileRegistry.components[i].elapsed_time += deltaTime;

        if (playerProjectileRegistry.components[i].elapsed_time > 5) {
            registry.remove_all_components_of(playerProjectileRegistry.entities[i]);
        }
    }

    UpdateWorldTexts(deltaTime);

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

    float playerExpPickUpRange = 38.f;
    if(registry.transforms.has(player))
    {
        auto& playerTransform = registry.transforms.get(player);
        for (Entity entity : registry.exp.entities) {
            // progress timer
            Exp& counter = registry.exp.get(entity);
            counter.counter_seconds_exp -= deltaTime;
            if (counter.counter_seconds_exp < 0.f) {
                registry.exp.remove(entity);
                registry.remove_all_components_of(entity);
            }

            if(!registry.transforms.has(entity) || !registry.motions.has(entity))
            {
                continue;
            }

            if(counter.counter_seconds_exp < (counter.counter_seconds_exp_default - 2.f))
            {
                auto& expTransform = registry.transforms.get(entity);
                auto& expMotion = registry.motions.get(entity);
                vec2 toPlayerVec = playerTransform.position - expTransform.position;
                if(length(toPlayerVec) < playerExpPickUpRange)
                {
                    expMotion.velocity = normalize(toPlayerVec) * 100.f;
                }
            }
        }
    }

    for (Entity entity : registry.coins.entities) {
        // progress timer
        Coin& counter1 = registry.coins.get(entity);
        counter1.counter_seconds_coin -= 2.f;

        if (counter1.counter_seconds_coin < 0.f) {
            registry.exp.remove(entity);
            registry.remove_all_components_of(entity);
        }
    }

    return true;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
    bool bGoToNextStage = false;

    Player &playerComponent = registry.players.get(player);
    TransformComponent &playerTransform = registry.transforms.get(player);
    MotionComponent &playerMotion = registry.motions.get(player);
    CollisionComponent &playerCollider = registry.colliders.get(player);
    HealthBar &playerHealth = registry.healthBar.get(player);
    GoldBar &playercoins = registry.goldBar.get(player);

    // Loop over all collisions detected by the physics system
    auto &collisionsRegistry = registry.collisionEvents;
    for (uint i = 0; i < collisionsRegistry.components.size(); i++) {

        const CollisionEvent colEvent = collisionsRegistry.components[i];
        Entity entity = collisionsRegistry.entities[i];
        Entity entity_other = colEvent.other;

        if (registry.enemy.has(entity)) {
            bool is_thrown_weapon = registry.items.has(entity_other)
                                    && registry.activePlayerProjectiles.has(entity_other)
                                    && (!registry.items.get(entity_other).grounded ||
                                        abs(registry.motions.get(entity_other).velocity.x) > 0);

            if (entity_other.GetTag() == TAG_PLAYERMELEEATTACK || is_thrown_weapon) {
                HealthBar &enemyHealth = registry.healthBar.get(entity);
                enemyHealth.TakeDamage((float) playerComponent.attackPower, (float) playerComponent.attackVariance);

                if (entity_other.GetTag() == TAG_PLAYERMELEEATTACK) {
                    // Move the player a little bit - its more fun
                    if (playerSystem->lastAttackDirection == 3) {
                        auto &playerMotion = registry.motions.get(player);
                        playerMotion.velocity.y = std::min(-playerMotion.velocity.y, -180.f);
                        playerMotion.velocity.x *= 1.7f;
                    } else if (playerSystem->lastAttackDirection == 0 || playerSystem->lastAttackDirection == 1) {
                        auto &playerMotion = registry.motions.get(player);
                        float bumpXVel = std::max(std::abs(playerMotion.velocity.x) * 1.5f, 150.f);
                        playerMotion.velocity.x = playerSystem->lastAttackDirection == 0 ? bumpXVel : -bumpXVel;
                    }
                }

                if (is_thrown_weapon)
                {
                    registry.activePlayerProjectiles.remove(entity_other);
                }

                if (enemyHealth.health <= 0.f && !registry.deathTimers.has(entity))
                {
                    registry.deathTimers.emplace(entity);
                    registry.colliders.remove(entity);
                    registry.collisionEvents.remove(entity);
                    MotionComponent &motion = registry.motions.get(entity);
                    motion.acceleration = {0.f, 0.f};
                    motion.velocity = {0.f, 0.f};

                    vec2 expPosition = registry.transforms.get(entity).position;
                    int coin_or_exp = RandomInt(0, 3);
                    if (coin_or_exp == 2) {
                        int random_count = RandomInt(1, 3);
                        for (int i = 1; i <= random_count; i++) 
                        {
                            createCoins(expPosition);
                        }
                    }
                    else 
                    {
                        int random_count = RandomInt(6, 8);
                        for (int i = 1; i <= random_count; i++) 
                        {
                            createExp(expPosition);
                        }
                    }

                    if (Mix_PlayChannel(-1, monster_death_sound, 0) == -1) {
                        printf("Mix_PlayChannel: %s\n", Mix_GetError());
                    }
                } 
                else {
                    if (Mix_PlayChannel(-1, monster_hurt_sound, 0) == -1) {
                        printf("Mix_PlayChannel: %s\n", Mix_GetError());
                    }
                }

                *GlobalPauseForSeconds = 0.1f;
            }
        }

        if (registry.players.has(entity)) {
            CheckCollisionWithBlockable(entity, entity_other);

            if (entity_other.GetTag() == TAG_SPIKE) {
                if (playerMotion.velocity.y > 0.f) // only hurt when falling on spikes
                {
                    playerHealth.health += -10.f;
                    if (Mix_PlayChannel(-1, player_hurt_sound, 0) == -1) {
                        printf("Mix_PlayChannel: %s\n", Mix_GetError());
                    }
                }
            }

            if (registry.enemy.has(entity_other)) {
                Enemy &enemy = registry.enemy.get(entity_other);

                // Check if player is moving vertically downwards
                if (playerMotion.velocity.y > 100.f && !(registry.flyingBehaviors.has(entity_other))) {
                    // player does not take damage & just bounces off
                    playerMotion.velocity = {0.f, -playerMotion.velocity.y};
                    if (Mix_PlayChannel(-1, player_jump_on_enemy_sound, 0) == -1) {
                        printf("Mix_PlayChannel: %s\n", Mix_GetError());
                    }
                } else if (enemy.playerHurtCooldown <= 0.f && playerHealth.health > 0.f && !(playerMotion.velocity.y > 0.f) && registry.meleeBehaviors.has(entity_other) && playerComponent.damageCooldown <= 0.f) {
                    const MeleeBehavior enemyMeleeBehavior = registry.meleeBehaviors.get(entity_other);
                    enemy.playerHurtCooldown = 2.f;
                    playerComponent.damageCooldown = 0.5f;
                    playerHealth.TakeDamage((float) enemyMeleeBehavior.attackPower, 5.f);
                    if (Mix_PlayChannel(-1, player_hurt_sound, 0) == -1) {
                        printf("Mix_PlayChannel: %s\n", Mix_GetError());
                    }
                }
            }

            if (registry.exp.has(entity_other))
            {
                int random_weight = RandomInt(6, 10);
                playerComponent.experience += random_weight;
                if (Mix_PlayChannel(-1, points_pickup_sound, 0) == -1) {
                    printf("Mix_PlayChannel: %s\n", Mix_GetError());
                }
                registry.remove_all_components_of(entity_other);

            }

            if (registry.coins.has(entity_other))
            {
                playercoins.coins += 10;
                if (Mix_PlayChannel(-1, coins_pickup_sound, 0) == -1) {
                    printf("Mix_PlayChannel: %s\n", Mix_GetError());
                }
                registry.remove_all_components_of(entity_other);

            }

            if (registry.shopItems.has(entity_other))
            {
                if (Input::HasKeyBeenPressed(SDL_SCANCODE_RETURN) && registry.activeShopItems.size() == 0) {
                    auto& activeItem = registry.activeShopItems.emplace(entity_other);
                    activeItem.linkedEntity.push_back(entity_other);
                }
            }

            if (registry.enemyProjectiles.has(entity_other)) {
                if (playerHealth.health > 0) {
                    const EnemyProjectile enemyProjectile = registry.enemyProjectiles.get(entity_other);
                    playerHealth.TakeDamage((float) enemyProjectile.attackPower, 5.f);
                    if (Mix_PlayChannel(-1, player_hurt_sound, 0) == -1) {
                        printf("Mix_PlayChannel: %s\n", Mix_GetError());
                    }
                }

                auto &enemyprojectileRegistry = registry.enemyProjectiles;
                Entity fire_entity = enemyprojectileRegistry.entities[0];
                registry.remove_all_components_of(fire_entity);
            }

            if (entity_other.GetTag() == TAG_LEVELENDPOINT && Input::GameUpHasBeenPressed()) {
                bGoToNextStage = true;
                continue;
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

        if (registry.exp.has(entity) || registry.coins.has(entity))
        {
            CheckCollisionWithBlockable(entity, entity_other);
        }

        if (registry.holders.has(entity)) {
            HolderComponent &holder = registry.holders.get(entity);

            if (entity_other.GetTagAndID() != 0 && entity_other != holder.held_weapon && registry.items.has(entity_other) && !registry.playerProjectiles.has(entity_other))
            {
                holder.near_weapon = entity_other;
            }
        }

        if(registry.items.has(entity))
        {
            if(registry.items.get(entity).collidableWithEnvironment)
            {
                if(!registry.playerProjectiles.has(entity) || registry.playerProjectiles.get(entity).elapsed_time > 0.01f)
                {
                    CheckCollisionWithBlockable(entity, entity_other, true, true);
                }

                if(entity_other.GetTag() == TAG_PLAYERBLOCKABLE)
                {
                    MotionComponent& itemMotion = registry.motions.get(entity);
                    float deceleration = 3.f;
                    deceleration = min(deceleration, abs(itemMotion.velocity.x));

                    if(registry.playerProjectiles.has(entity))
                    {
                        auto& _proj = registry.playerProjectiles.get(entity);
                        if(_proj.bHitWall || _proj.elapsed_time >= _proj.minTravelTime)
                        {
                            if(itemMotion.velocity.x > 0) 
                            {
                                itemMotion.velocity.x -= deceleration;
                            } 
                            else if(itemMotion.velocity.x < 0) 
                            {
                                itemMotion.velocity.x += deceleration;
                            }
                        }
                    }
                    else
                    {
                        if(itemMotion.velocity.x > 0) 
                        {
                            itemMotion.velocity.x -= deceleration;
                        } 
                        else if(itemMotion.velocity.x < 0) 
                        {
                            itemMotion.velocity.x += deceleration;
                        }
                    }
                }
            }
        }
    }
    // Remove all collisions from this simulation Step
    registry.collisionEvents.clear();

    if (playerHealth.health <= 0.f && !playerComponent.bDead) {
        // DEAD
        if (Mix_PlayChannel(-1, player_death_sound, 0) == -1) {
            printf("Mix_PlayChannel: %s\n", Mix_GetError());
        }
        playerComponent.bDead = true;
        darkenGameFrame = true;
        *GlobalPauseForSeconds = 3.f;
    }

    if(bGoToNextStage)
    {
        StartNewStage((GAMELEVELENUM) ((u8) currentGameStage + 1));
    }
}

void WorldSystem::CheckCollisionWithBlockable(Entity entity_resolver, Entity entity_other, bool bounce_x, bool is_item) {
    if (entity_other.GetTag() == TAG_PLAYERBLOCKABLE) {
        if (registry.colliders.has(entity_resolver) && registry.colliders.has(entity_other)) {
            CollisionComponent &resolverCollider = registry.colliders.get(entity_resolver);
            CollisionComponent &otherCollider = registry.colliders.get(entity_other);

            /** Note(Kevin): This collisionCheckAgain is required because as we resolve collisions
             *  by moving entities around, the initial collection of collision events may become outdated.
             *  Checking that the two entities are still colliding is not a perfect solution (if there
             *  even is one), but it should be good enough... We can revisit this and attempt other
             *  solutions down the line if needed. */
            CollisionInfo collisionCheckAgain = CheckCollision(resolverCollider, otherCollider);
            if (collisionCheckAgain.collides) {
                TransformComponent &resolverTransform = registry.transforms.get(entity_resolver);
                if (abs(collisionCheckAgain.collision_overlap.x) < abs(collisionCheckAgain.collision_overlap.y)) {
                    MotionComponent &resolverMotion = registry.motions.get(entity_resolver);
                    resolverTransform.position.x += collisionCheckAgain.collision_overlap.x;
                    resolverCollider.collider_position.x += collisionCheckAgain.collision_overlap.x;

                    if (bounce_x) {
                        resolverMotion.velocity.x = 0.25f * -resolverMotion.velocity.x;
                    } 
                    else
                    {
                        resolverMotion.velocity.x = 0.f;
                    }

                    if(is_item)
                    {
                        if(registry.playerProjectiles.has(entity_resolver))
                        {
                            auto& _proj = registry.playerProjectiles.get(entity_resolver);
                            _proj.bHitWall = true;
                        }
                    }
                }
                else
                {
                    resolverTransform.position.y += collisionCheckAgain.collision_overlap.y;
                    resolverCollider.collider_position.y += collisionCheckAgain.collision_overlap.y;
                    if (collisionCheckAgain.collision_overlap.y > 1) {
                        MotionComponent& resolverMotion = registry.motions.get(entity_resolver);
                        resolverMotion.velocity.y = 0;
                    }
                    

                    if (is_item && collisionCheckAgain.collision_overlap.y > 0) {
                        registry.items.get(entity_resolver).grounded = true;
                    }
                }
            }
        }
    }
}

void WorldSystem::UpdateWorldTexts(float dt)
{
    if(!registry.transforms.has(player))
    {
        return;
    }
        
    TransformComponent& playerTransform = registry.transforms.get(player);
    for(int i = 0; i < registry.proximityTexts.size(); ++i)
    {
        ProximityTextComponent& proximText = registry.proximityTexts.components[i];
        if(length(proximText.triggerPosition - playerTransform.position) < proximText.triggerRadius)
        {
            proximText.__bCurrentlyTriggered = true;

            if(proximText.bTyped)
            {
                proximText.__typingTimer += dt;
                if(proximText.__typingTimer > proximText.secondsBetweenTypedCharacters)
                {
                    proximText.__typingTimer = 0.f;
                    if(proximText.__currentTypedTextBuffer.size() < proximText.text.size())
                    {
                        char cat = proximText.text.at(proximText.__currentTypedTextBuffer.size());
                        proximText.__currentTypedTextBuffer.push_back(cat);
                    }
                }
            }
            else
            {
                proximText.__currentTypedTextBuffer = proximText.text;
            }

            uiSystem->PushWorldText(proximText.textPosition, proximText.__currentTypedTextBuffer, proximText.textSize);
        }
        else
        {
            proximText.__bCurrentlyTriggered = false;
            proximText.__currentTypedTextBuffer = "";
        }
    }
}

// Should the game be over ?
bool WorldSystem::is_over() const {
    return !gameIsRunning;
}

void WorldSystem::SDLProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN: {
                Input::ProcessSDLKeyDownEvent(event.key);

                if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    console_toggle();
                    break;
                }

                if (console_is_shown())
                {
                    console_keydown(event.key);
                    break;
                }
            }break;

            case SDL_KEYUP: {
                Input::ProcessSDLKeyUpEvent(event.key);
            }
                break;

            case SDL_CONTROLLERBUTTONDOWN: {
                Input::ProcessSDLControllerButtonDownEvent(event.cbutton);
            }
                break;
            case SDL_CONTROLLERBUTTONUP: {
                Input::ProcessSDLControllerButtonUpEvent(event.cbutton);
            }
                break;
            case SDL_CONTROLLERAXISMOTION: {
                Input::ProcessSDLControllerAxisEvent(event.caxis);
            }
                break;
            case SDL_CONTROLLERDEVICEADDED: {
                Input::SDLControllerConnected(event.cdevice.which);
            }
                break;
            case SDL_CONTROLLERDEVICEREMOVED: {
                Input::SDLControllerRemoved(event.cdevice.which);
            }
                break;

            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        renderer->UpdateBackBufferSize();
                    }
                        break;
                }
            }
                break;

            case SDL_QUIT: {
                gameIsRunning = false;
            }
                break;
        }
    }
}

void swapPlayerStats() {

}
