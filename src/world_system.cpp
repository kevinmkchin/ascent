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

// Put game configuration stuff here maybe
INTERNAL Entity player;
INTERNAL Entity enemy1;

// Create the bug world
WorldSystem::WorldSystem()
	: points(0)
	, next_eagle_spawn(0.f)
	, next_bug_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

void WorldSystem::init(RenderSystem* renderer_arg)
{
    loadAllContent();

	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

void WorldSystem::cleanUp()
{
    unloadAllContent();
}

void WorldSystem::loadAllContent()
{
    background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
    chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
    chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());

    if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr) {
        fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
                audio_path("music.wav").c_str(),
                audio_path("chicken_dead.wav").c_str(),
                audio_path("chicken_eat.wav").c_str());
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
    Mix_CloseAudio();

    // Destroy all created components
    registry.clear_all_components();
}

// Update our game world
bool WorldSystem::step(float deltaTime) {

	// Remove debug info from the last Step
	while (registry.debugComponents.entities.size() > 0)
        registry.remove_all_components_of(registry.debugComponents.entities.back());

    if(Input::HasKeyBeenPressed(SDL_SCANCODE_R))
    {
        restart_game();
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

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

    srand((u32) time(nullptr));
    u32 seed = rand()%1000000000;
    GenerateNewLevel(seed);
    renderer->cameraBoundMin = currentLevelData.cameraBoundMin;
    renderer->cameraBoundMax = currentLevelData.cameraBoundMax;

    player = createBox(currentLevelData.playerStart);
    registry.players.emplace(player);
	HealthBar& playerHealth = registry.healthBar.get(player);
	printf("%f \n", registry.healthBar.get(player).health);
	playerHealth.health = 2000.f;
	printf("%f \n", playerHealth.health);
	enemy1 = createEnemy(vec2(238.f, 136.f));
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisionEvents;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
        const CollisionEvent colEvent = collisionsRegistry.components[i];
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = colEvent.other;

		// For now, we are only interested in collisions that involve the chicken
		if (registry.players.has(entity)) {
            Player& player = registry.players.get(entity);
			HealthBar& hb = registry.healthBar.get(entity);
            Motion& playerMotion = registry.motions.get(entity);

			if (registry.enemy.has(entity_other)) {
				if (hb.health > 0) {
					hb.health -= 20;
				}
				else {
					//Mix_PlayChannel(-1, chicken_dead_sound, 0);
				}
				printf("Colliding with enemy. Reduced health to: %f \n", hb.health);
			}

            /** Note(Kevin): This collisionCheckAgain is required because as we resolve collisions
             *  by moving entities around, the initial collection of collision events may become outdated.
             *  Checking that the two entities are still colliding is not a perfect solution (if there
             *  even is one), but it should be good enough... We can revisit this and attempt other
             *  solutions down the line if needed. */
            CollisionInfo collisionCheckAgain = CheckCollision(playerMotion, registry.motions.get(entity_other));
            if(collisionCheckAgain.collides)
            {
                if(abs(collisionCheckAgain.collision_overlap.x) < abs(collisionCheckAgain.collision_overlap.y))
                {
                    playerMotion.position.x += collisionCheckAgain.collision_overlap.x;
                }
                else
                {
                    playerMotion.position.y += collisionCheckAgain.collision_overlap.y;
                }
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
	}
	// Remove all collisions from this simulation Step
	registry.collisionEvents.clear();
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
                        renderer->updateBackBufferSize();
                    }break;
                }
            }break;

            case SDL_QUIT:{
                gameIsRunning = false;
            }break;
        }
    }
}
