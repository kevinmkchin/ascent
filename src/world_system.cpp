// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
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
        playerMotion.velocity.y = -playerMoveSpeed;
    }
    if(Input::IsKeyPressed(SDL_SCANCODE_S) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_DOWN))
    {
        playerMotion.velocity.y = playerMoveSpeed;
    }
    if(Input::IsKeyPressed(SDL_SCANCODE_A) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_LEFT))
    {
        playerMotion.velocity.x = -playerMoveSpeed;
    }
    if(Input::IsKeyPressed(SDL_SCANCODE_D) || Input::GetGamepad(0).IsPressed(GAMEPAD_DPAD_RIGHT))
    {
        playerMotion.velocity.x = playerMoveSpeed;
    }
    if(Input::GetGamepad(0).isConnected)
    {
        playerMotion.velocity = Input::GetGamepad(0).leftThumbStickDir * playerMoveSpeed;
    }

    if (Input::HasKeyBeenReleased(SDL_SCANCODE_W) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_DPAD_UP))
    {
        playerMotion.velocity.y = 0;
    }
    if (Input::HasKeyBeenReleased(SDL_SCANCODE_S) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_DPAD_DOWN))
    {
        playerMotion.velocity.y = 0;
    }
    if (Input::HasKeyBeenReleased(SDL_SCANCODE_A) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_DPAD_LEFT))
    {
        playerMotion.velocity.x = 0;
    }
    if (Input::HasKeyBeenReleased(SDL_SCANCODE_D) || Input::GetGamepad(0).HasBeenReleased(GAMEPAD_DPAD_RIGHT))
    {
        playerMotion.velocity.x = 0;
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

    player = createBox(vec2(128.f, 96.f));
    registry.players.emplace(player);
	HealthBar& playerHealth = registry.healthBar.get(player);
	printf("%f \n", registry.healthBar.get(player).health);
	playerHealth.health = 2000.f;
	printf("%f \n", playerHealth.health);
	enemy1 = createEnemy(vec2(230.f, 128.f));

    CreateBasicLevelTile(1,9);
    CreateBasicLevelTile(2,9);
    CreateBasicLevelTile(3,9);
    CreateBasicLevelTile(4,9);
    CreateBasicLevelTile(5,9);
    CreateBasicLevelTile(6,9);
    CreateBasicLevelTile(7,9);
    CreateBasicLevelTile(8,9);
    CreateBasicLevelTile(9,9);
    CreateBasicLevelTile(10,9);
    CreateBasicLevelTile(11,9);
    CreateBasicLevelTile(12,9);
    CreateBasicLevelTile(13,9);
    CreateBasicLevelTile(14,9);
    CreateBasicLevelTile(15,9);
    CreateBasicLevelTile(16,9);
    CreateBasicLevelTile(17,9);
    CreateBasicLevelTile(18,9);
    CreateBasicLevelTile(3,8);
    CreateBasicLevelTile(4,8);
    CreateBasicLevelTile(7,8);
    CreateBasicLevelTile(8,8);
    CreateBasicLevelTile(9,8);
    CreateBasicLevelTile(8,7);
    CreateBasicLevelTile(13,5);
    CreateBasicLevelTile(14,5);
    CreateBasicLevelTile(15,5);
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
        const Collision colEvent = collisionsRegistry.components[i];
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
