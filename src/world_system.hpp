#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#include <SDL.h>
#include <SDL_mixer.h>

class RenderSystem;
class PlayerSystem;

enum GAMEMODE
{
	MODE_MAINMENU,
	MODE_INGAME
};

enum GAMELEVELENUM : u8
{
	CHAPTER_ONE_STAGE_ONE,
	CHAPTER_ONE_STAGE_TWO,
	// CHAPTER_ONE_STAGE_THREE,
	// CHAPTER_TWO_STAGE_ONE,
	// CHAPTER_TWO_STAGE_TWO,
	// CHAPTER_TWO_STAGE_THREE,
	// CHAPTER_THREE_STAGE_ONE,
	// CHAPTER_THREE_STAGE_TWO,
	// CHAPTER_THREE_STAGE_THREE,
	END_THE_GAME,

	GAME_NOT_STARTED,
};

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// starts the game
	void init(RenderSystem* renderer_arg, PlayerSystem* player_sys_arg);

    void cleanUp();

    void StartNewRun();

    void UpdateMode();

	// Steps the game ahead by deltaTime
	bool step(float deltaTime);

	// Check for collisions
	void handle_collisions();

    // Handle input events
    void SDLProcessEvents();

    GAMEMODE GetCurrentMode() { return currentGameMode; }

	bool is_over()const;

    void set_is_over(bool over) { gameIsRunning = over; }

private:
    void loadAllContent();

    void unloadAllContent();

	void StartNewStage(GAMELEVELENUM stage);

    void SetCurrentMode(GAMEMODE mode);

    void CheckCollisionWithBlockable(Entity entity_resolver, Entity entity_other);

	// OpenGL window handle
	SDL_Window* window;

	// Game state
    bool gameIsRunning;
    GAMEMODE currentGameMode;
    GAMELEVELENUM currentGameStage;
	RenderSystem* renderer;
	PlayerSystem* playerSystem;
	Entity player;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* chicken_dead_sound;
	Mix_Chunk* chicken_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
