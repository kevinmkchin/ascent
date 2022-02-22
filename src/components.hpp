#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include <array>
#include <stb_image.h>

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next Draw command.
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

enum class TEXTURE_ASSET_ID : u16 
{
	EAGLE,
    MONSTER,
    TOPTILE1,
    MIDTILE1,
    BOTTILE1,
    TOPBOTTILE1,
    SKULLS1,
    SKULLS2,
    SPIKES1,
    SPIKES2,
    BG1,
    BOX,
    LADDER,
    WOODTILE,
    TILE_EXAMPLE,
    SHOPBG,
    MAINMENUBG,
    FIRE,
    EXITTILE,

    TEXTURE_COUNT
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

// Make sure these paths remain in sync with the associated enumerators.
const std::array<std::string, texture_count> texture_paths = {
        textures_path("eagle.png"),
        textures_path("monster.png"),
        textures_path("toptile.png"),
        textures_path("midtile.png"),
        textures_path("bottile.png"),
        textures_path("topbottile.png"),
        textures_path("skulls1.png"),
        textures_path("skulls2.png"),
        textures_path("spikes1.png"),
        textures_path("spikes2.png"),
        textures_path("bg.png"),
        textures_path("box.png"),
        textures_path("ladder.png"),
        textures_path("woodtile.png"),
        textures_path("tile_example.png"),
        textures_path("shopbg.png"),
        textures_path("mainmenu.png"),
        textures_path("fire.png"),
        textures_path("exit.png"),
};

enum class EFFECT_ASSET_ID : u8
{
    FINAL_PASS,
    SPRITE,
    BACKGROUND,
    TEXT,
    EXP_UI,

    EFFECT_COUNT
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

// Make sure these paths remain in sync with the associated enumerators.
const std::array<std::string, effect_count> effect_paths = {
        shader_path("finalpass"),
        shader_path("sprite"),
        shader_path("background"),
        shader_path("text_ui"),
        shader_path("exp_ui"),
};


/**
 *
 * Components
 *
 * */

// Player component
struct Player
{
    i32 attackPower = 3;
    u8  level = 1;
    float experience = 0.f;
};
const float PLAYER_EXP_THRESHOLDS_ARRAY[10] = { 0.f, 100.f, 300.f, 700.f, 1500.f, 9999.f, 9999.f, 9999.f, 9999.f, 9999.f }; 

struct Enemy
{
    float projectile_speed = 120.f;
};

struct Enemy_projectile {
    Entity enemy_projectile;
};

struct TransformComponent
{
    vec2 position = { 0.f, 0.f };
    vec2 scale = { 1.f, 1.f };
    vec2 center = { 0.f, 0.f }; // offset from "top-left". If a sprite has dimensions 16x16, then center of 8x8 points to center of sprite
    float rotation = 0.f;
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
    vec2 dimensions = {0.f,0.f};// in pixels
    i8 layer = 0;               // render layer. higher layer is drawn on top of lower layers. -128 to 127
    TEXTURE_ASSET_ID texId = TEXTURE_ASSET_ID::EAGLE;     // ID for the texture to use
    EFFECT_ASSET_ID shaderId = EFFECT_ASSET_ID::SPRITE;   // ID for the shader to use
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

struct Mutation {
    std::string name;
    int velocityEffect;
    int attackPowerEffect;
    int healthEffect;
    SpriteComponent sprite;
};

struct MutationComponent {
    Mutation* currentActiveMutations [5] = {};
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

enum GAMETAGS : u8
{
    TAG_PLAYER,
    TAG_LEVELENDPOINT,
    TAG_LADDER,
    TAG_SPIKE,
    TAG_PLAYERBLOCKABLE,
};
