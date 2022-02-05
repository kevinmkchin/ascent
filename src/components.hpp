#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include <array>
#include "../ext/stb_image/stb_image.h"

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
    BUG = 0,
	EAGLE = BUG + 1,
    SPELUNKY = EAGLE + 1,
    MONSTER = SPELUNKY + 1,
    TILE_EXAMPLE = MONSTER + 1,
    BOX = TILE_EXAMPLE + 1,
    TEXTURE_COUNT = BOX + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
    FINAL_PASS = 0,
    SPRITE = 1,
    EFFECT_COUNT = SPRITE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

// Make sure these paths remain in sync with the associated enumerators.
const std::array<std::string, texture_count> texture_paths = {
        textures_path("bug.png"),
        textures_path("eagle.png"),
        textures_path("spelunky.png"),
        textures_path("monster.png"),
        textures_path("tile_example.png"),
        textures_path("box.png"),
};

// Make sure these paths remain in sync with the associated enumerators.
const std::array<std::string, effect_count> effect_paths = {
        shader_path("finalpass"),
        shader_path("sprite"),
};


/**
 *
 * Components
 *
 * */

// Player component
struct Player
{

};

// Eagles have a hard shell
struct Deadly
{

};

// Bug and Chicken have a soft shell
struct Eatable
{

};

struct Enemy
{

};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = { 0, 0 };
	float rotation = 0;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 1, 1 };
    vec2 center = { 0, 0 };

    // Collision box x,y size in the positive direction from the center
    vec2 collision_pos = { 0, 0 };
    // Collision box x,y size in the negative direction from the center
    vec2 collision_neg = { 0, 0 };
    float jumpRequest = 0; // requesting a jump with this speed
};

struct SpriteComponent
{
    // TODO(Kevin)

    vec2 dimensions; // in pixels
    TEXTURE_ASSET_ID texId;
    // some ID for the texture we want to use

    // Texture we want to use
    // Dimensions of that texture
    // Info about which region of the texture to use as sprite
};

// Stucture to store collision information
struct Collision
{
    vec2 collision_overlap = {0, 0};
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
	float counter_ms = 3000;
};

struct HealthBar
{
	float health = 0;
};
