#pragma once

// internal
#include "common.hpp"
#include "components.hpp"
#include "ai_system.hpp"

// stlib
#include <vector>
#include <random>

#include <SDL.h>
#include <SDL_mixer.h>

#define TILE_SIZE 16
#define ROOM_DIMENSION_X 11
#define ROOM_DIMENSION_Y 9
#define NUMROOMSWIDE 5
#define NUMFLOORS 5
#define NUMTILESWIDE NUMROOMSWIDE * ROOM_DIMENSION_X
#define NUMTILESTALL NUMFLOORS * ROOM_DIMENSION_Y

class RenderSystem;
class PlayerSystem;
class UISystem;

enum GAMEMODE
{
	MODE_MAINMENU,
	MODE_INGAME,
	MODE_HELP,
	MODE_HELP1
};

enum DIFFICULTY
{
	DIFFICULTY_STANDARD,
	DIFFICULTY_EASY
};

enum GAMELEVELENUM : u8
{
	CHAPTER_TUTORIAL,
	CHAPTER_ONE_STAGE_ONE,
	//CHAPTER_ONE_STAGE_TWO,
	//CHAPTER_ONE_STAGE_THREE,
	CHAPTER_TWO_STAGE_ONE,
	// CHAPTER_TWO_STAGE_TWO,
	// CHAPTER_TWO_STAGE_THREE,
	CHAPTER_THREE_STAGE_ONE,
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
	void init(RenderSystem* renderer_arg, PlayerSystem* player_sys_arg, AISystem* ai_sys_arg);

    void cleanUp();

    void StartNewRun();

    void UpdateMode();

	std::vector<std::vector<int>> levelForAI();

	// Steps the game ahead by deltaTime
	bool step(float deltaTime);

	// Check for collisions
	void handle_collisions();

	void HandleMutations();

    // Handle input events
    void SDLProcessEvents();

    GAMEMODE GetCurrentMode() { return currentGameMode; }
    GAMELEVELENUM GetCurrentStage() { return currentGameStage; }
	DIFFICULTY GetCurrentDifficulty() { return currentDifficulty; }
	void SwapCurrentDifficulty() {
		if (currentDifficulty == DIFFICULTY_STANDARD) {
			currentDifficulty = DIFFICULTY_EASY;
		}
		else {
			currentDifficulty = DIFFICULTY_STANDARD;
		}
	}

	void HelpMode();

	bool is_over()const;


    void set_is_over(bool over) { gameIsRunning = over; }

    float* GlobalPauseForSeconds = nullptr;
    bool darkenGameFrame = false;
    bool gamePaused = false;

    std::vector<Mutation> allPossibleMutations;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* chicken_dead_sound;
	Mix_Chunk* chicken_eat_sound;
	Mix_Chunk* sword_sound;
	Mix_Chunk* monster_hurt_sound;
	Mix_Chunk* monster_death_sound;
	Mix_Chunk* player_hurt_sound;
	Mix_Chunk* player_death_sound;
	Mix_Chunk* gain_mutation_sound;
	Mix_Chunk* player_levelup_sound;
	Mix_Chunk* blip_select_sound;
    Mix_Chunk* player_jump_on_enemy_sound; 
	Mix_Chunk* coins_pickup_sound;
	Mix_Chunk* points_pickup_sound;

	// HACK should prob remove later
	UISystem* uiSystem;

private:
    void loadAllContent();

    void unloadAllContent();

	void StartNewStage(GAMELEVELENUM stage);

	void SpawnLevelEntities();

    void SetCurrentMode(GAMEMODE mode);

    void CheckCollisionWithBlockable(Entity entity_resolver, Entity entity_other, bool bounce_x = false, bool is_item = false);

    void UpdateWorldTexts(float dt);

	// OpenGL window handle
	SDL_Window* window;

	// Game state
    bool gameIsRunning;
    GAMEMODE currentGameMode;
    GAMELEVELENUM currentGameStage;
	DIFFICULTY currentDifficulty = DIFFICULTY_EASY;
	RenderSystem* renderer;
	PlayerSystem* playerSystem;
	AISystem* aiSystem;
	Entity player;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
