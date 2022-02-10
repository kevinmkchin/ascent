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
    TOPTILE1 = MONSTER + 1,
    MIDTILE1 = TOPTILE1 + 1,
    BOTTILE1 = MIDTILE1 + 1,
    TOPBOTTILE1 = BOTTILE1 + 1,
    BG1 = TOPBOTTILE1 + 1,
    TILE_EXAMPLE = BG1 + 1,
    BOX = TILE_EXAMPLE + 1,
    TEXTURE_COUNT = BOX + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
    FINAL_PASS = 0,
    SPRITE = 1,
    BACKGROUND = 2,
    EFFECT_COUNT = BACKGROUND + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

// Make sure these paths remain in sync with the associated enumerators.
const std::array<std::string, texture_count> texture_paths = {
        textures_path("bug.png"),
        textures_path("eagle.png"),
        textures_path("spelunky.png"),
        textures_path("monster.png"),
        textures_path("toptile.png"),
        textures_path("midtile.png"),
        textures_path("bottile.png"),
        textures_path("topbottile.png"),
        textures_path("bg.png"),
        textures_path("tile_example.png"),
        textures_path("box.png"),
};

// Make sure these paths remain in sync with the associated enumerators.
const std::array<std::string, effect_count> effect_paths = {
        shader_path("finalpass"),
        shader_path("sprite"),
        shader_path("background"),
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

struct Enemy
{

};

struct TransformComponent
{
    vec2 position = { 0.f, 0.f };
    float rotation = 0.f;
    vec2 scale = { 1.f, 1.f };
    vec2 center = { 0.f, 0.f }; // offset from "top-left". If a sprite has dimensions 16x16, then center of 8x8 points to center of sprite
};

// All data relevant to the motion of entities
struct MotionComponent
{
    vec2 velocity = { 0.f, 0.f };               // signed
    vec2 acceleration = { 0.f, 0.f };           // signed
    vec2 terminalVelocity = { 9999.f, 9999.f }; // unsigned
};

struct CollisionComponent
{
    // Collision
    vec2 collision_pos = { 0.f, 0.f }; // Collision box x,y size in the positive direction from the center
    vec2 collision_neg = { 0.f, 0.f }; // Collision box x,y size in the negative direction from the center
};

struct SpriteComponent
{
    vec2 dimensions; // in pixels
    TEXTURE_ASSET_ID texId; // ID for the texture we want to use
    // TODO(Kevin): Info about which region of the texture to use as sprite
};

struct CollisionEvent
{
    // Stucture to store collision information
    vec2 collision_overlap = {0, 0};
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	CollisionEvent(Entity& other) { this->other = other; };
};

struct LevelGeometryComponent
{

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

struct HealthBar
{
	float health = 0;
};
