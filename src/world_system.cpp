// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include "input.hpp"

// Game configuration
const size_t MAX_EAGLES = 15;
const size_t MAX_BUG = 5;
const size_t EAGLE_DELAY_MS = 2000 * 3;
const size_t BUG_DELAY_MS = 5000 * 3;

INTERNAL Entity player;

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

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

    // NOTE(Kevin): this is all temporary - feel free to delete
    Motion& playerMotion = registry.motions.get(player);
    float playerMoveSpeed = 64.f;
    if(Input::IsKeyPressed(SDL_SCANCODE_W) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_UP))
    {
        playerMotion.position.y -= playerMoveSpeed * deltaTime;
    }
    if(Input::IsKeyPressed(SDL_SCANCODE_S) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_DOWN))
    {
        playerMotion.position.y += playerMoveSpeed * deltaTime;
    }
    if(Input::IsKeyPressed(SDL_SCANCODE_A) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_LEFT))
    {
        playerMotion.position.x -= playerMoveSpeed * deltaTime;
    }
    if(Input::IsKeyPressed(SDL_SCANCODE_D) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_RIGHT))
    {
        playerMotion.position.x += playerMoveSpeed * deltaTime;
    }
    if(Input::GetGamepad(0).isConnected)
    {
        playerMotion.position += Input::GetGamepad(0).leftThumbStickDir * playerMoveSpeed * deltaTime;
    }


    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
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
	}

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

    createBox(vec2(100.f,50.f)); // TODO(Kevin): remove this later - just for testing
    player = createBox(vec2(128.f, 128.f));
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// For now, we are only interested in collisions that involve the chicken
		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity)) {
					// Scream, reset timer, and make the chicken sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, chicken_dead_sound, 0);
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, chicken_eat_sound, 0);
					++points;
				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
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
