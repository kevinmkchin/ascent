#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include <array>
#include <functional>
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
    ASCENT_LEVELTILES_SHEET,
    ASCENT_MUTATIONS_SHEET,
    MONSTER,
    BOSS_MELEE,
    BOSS_RAGE,
    BOSS_RANGED,
    BOSS_PROJECTILE,
    BOSS_PROJECTILE_REVERSE,
    BG1,
    BOX,
    SHOPBG,
    MAINMENUBG,
    FIRE,
    LOB_PROJECTILE,
    PLAYER,
    SWORD,
    BG_LAYER1,
    BG_LAYER2,
    BG_LAYER3,
    BG_LAYER4,
    BG_LAYER5,
    GOBLIN,
    BOW_AND_ARROW,
    FLYING_ENEMY,
    GOBLIN_BOMBER,
    MUSHROOM,
    SLIME,
    WORM,
    EXP,
    COIN,
    TORCH,
    SWORDSWING_LEFT,
    SWORDSWING_RIGHT,
    SWORDSWING_UP,
    SWORDSWING_DOWN,
    WOODENSIGNS,
    BG_MENU_LAYER1,
    BG_MENU_LAYER2,
    BG_MENU_LAYER3,
    BG_MENU_LAYER4,
    BG_MENU_LAYER5,
    BG_VILLAGE_LAYER1,
    BG_VILLAGE_LAYER2,
    BG_VILLAGE_LAYER3,
    BG_VILLAGE_LAYER4,
    BG_VILLAGE_LAYER5,
    BG_VILLAGE_LAYER6,
    BG_VILLAGE_LAYER7,
    BG_VILLAGE_LAYER8,
    BG_VILLAGE_LAYER9,
    BG_VILLAGE_LAYER10,
    BG_FOREST_LAYER1,
    BG_FOREST_LAYER2,
    BG_FOREST_LAYER3,
    BG_FOREST_LAYER4,
    BG_MOUNTAIN_LAYER1,
    BG_MOUNTAIN_LAYER2,
    BG_MOUNTAIN_LAYER3,
    BG_MOUNTAIN_LAYER4,
    BG_MOUNTAIN_LAYER5,
    BG_MOUNTAIN_LAYER6,
    BG_MOUNTAIN_LAYER7,
    BG_MOUNTAIN_LAYER8,
    HELP_MENU,
    CREDITS,
    HEALTH_POTION,
    TILES_CAVE,
    TILES_FOREST,
    TILES_SNOWY,
    TILES_DECORATIONS,
    WALKING_BOMB,

    TEXTURE_COUNT
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

// Make sure these paths remain in sync with the associated enumerators.
const std::array<std::string, texture_count> texture_paths = {
        textures_path("ascent_leveltiles_spritesheet.png"),
        textures_path("mutations_spritesheet.png"),
        textures_path("monster.png"),
        textures_path("boss_melee.png"),
        textures_path("boss_rage.png"),
        textures_path("boss_ranged.png"),
        textures_path("boss_projectile.png"),
        textures_path("boss_projectile_reverse.png"),
        textures_path("bg.png"),
        textures_path("box.png"),
        textures_path("shopbg.png"),
        textures_path("mainmenu.png"),
        textures_path("fire.png"),
        textures_path("lob_projectile.png"),
        textures_path("player.png"),
        textures_path("sword_swing_smaller.png"),
        textures_path("bg_layer1.png"),
        textures_path("bg_layer2.png"),
        textures_path("bg_layer3.png"),
        textures_path("bg_layer4.png"),
        textures_path("bg_layer5.png"),
        textures_path("goblin.png"),
        textures_path("bow_and_arrow.png"),
        textures_path("batEnemy.png"),
        textures_path("goblin_bomber.png"),
        textures_path("mushroom.png"),
        textures_path("slime_fixed.png"),
        textures_path("worm.png"),
        textures_path("exp.png"),
        textures_path("coin.png"),
        textures_path("torch.png"),
        textures_path("swing_left.png"),
        textures_path("swing_right.png"),
        textures_path("swing_up.png"),
        textures_path("swing_down.png"),
        textures_path("signs.png"),
        textures_path("menu_bg_layer1.png"),
        textures_path("menu_bg_layer2.png"),
        textures_path("menu_bg_layer3.png"),
        textures_path("menu_bg_layer4.png"),
        textures_path("menu_bg_layer5.png"),
        textures_path("village background/Background.png"),
        textures_path("village background/1.png"),
        textures_path("village background/2.png"),
        textures_path("village background/3.png"),
        textures_path("village background/4.png"),
        textures_path("village background/5.png"),
        textures_path("village background/6.png"),
        textures_path("village background/7.png"),
        textures_path("village background/8.png"),
        textures_path("village background/Foreground.png"),
        textures_path("forest background/parallax-demon-woods-bg.png"),
        textures_path("forest background/parallax-demon-woods-far-trees.png"),
        textures_path("forest background/parallax-demon-woods-mid-trees.png"),
        textures_path("forest background/parallax-demon-woods-close-trees.png"),
        textures_path("mountain background/1.png"),
        textures_path("mountain background/2.png"),
        textures_path("mountain background/3.png"),
        textures_path("mountain background/4.png"),
        textures_path("mountain background/5.png"),
        textures_path("mountain background/6.png"),
        textures_path("mountain background/7.png"),
        textures_path("mountain background/8.png"),
        textures_path("help.png"),
        textures_path("credits.png"),
        textures_path("health_potion.png"),
        textures_path("tiles_cave.png"),
        textures_path("tiles_forest.png"),
        textures_path("tiles_snowy.png"),
        textures_path("tiles_decorations.png"),
        textures_path("walking_bomb_small.png"),
};

enum class EFFECT_ASSET_ID : u8
{
    FINAL_PASS,
    SPRITE,
    BACKGROUND,
    TEXT,
    EXP_UI,
    HEALTHBAR_UI,
    HEALTHBAR_BORDER_UI,
    MUTATION_SELECT_UI,
    CONSOLE_UI,
    CONSOLE_TEXT_UI,
    WORLDTEXT,

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
        shader_path("healthbar_ui"),
        shader_path("healthbar_border_ui"),
        shader_path("mutation_select_ui"),
        shader_path("console_ui"),
        shader_path("console_text_ui"),
        shader_path("worldtext")
};

/**
 *
 * Components
 *
 * */

// Player component
struct Player
{
    bool bDead = false;
    u8  level = 1;
    float experience = 0.f;

    float movementSpeed = 64.f;
    i32 attackPower = 30;
    i32 attackVariance = 10;
    i16 meleeAttackRange = 16;
    i16 meleeAttackArc = 12;
    float meleeAttackCooldown = 0.8f;  
    float damageCooldown = 0.0f;

    i32 maxJumps = 1;
};
const float PLAYER_EXP_THRESHOLDS_ARRAY[10] = { 0.f, 100.f, 300.f, 700.f, 1200.f, 1850.f, 2650.f, 3700.f, 5000.f, 9999.f }; 

struct Enemy
{
    float projectile_speed = 100.f;
    float playerHurtCooldown = 0.f;
    //std::vector<Behavior> behaviors;
};

struct Boss
{
    i32 meleeAttackPower  = 10;
    i32 rangedAttackPower =  7;

    bool summonState  = false;
    bool meleeState   = false;
    bool rangedState  = false;
    bool rageState    = false;
    bool isBuffering  = false;
    bool firstAttack  = false;
    bool secondAttack = false;

    float actionTimer    = 0;
    float actionCooldown = 0;
    float actionBuffer   = 0;
    int actionTick       = 0;

    int rageTick = 25;
    bool facingRight = false;
};

struct Behavior {

};

struct EnemyProjectile {
    Entity enemy_projectile;
    i32 attackPower = 0;
};

struct EnemyMeleeAttack {
    i32 attackPower = 0;
    float elapsedTime = 0;
    float existenceTime = 0;
};

struct Exp
{
    float counter_seconds_exp_default = 6.f;
    float counter_seconds_exp = counter_seconds_exp_default;
};

struct Coin
{
    float counter_seconds_coin = 6.f;
};

struct HealthPotion
{
    float counter_seconds_health = 6.f;
    float healthRestoreAmount = 5.f;
};

struct PathingBehavior {
    vec2 goalFromPlayer = { 0.f, 0.f }; // (absolute value?) distance from player enemy would ideally like to be (in (x,y))
    float pathSpeed = 0;
};

struct PatrollingBehavior : Behavior {
    bool standStill; 
    float patrolSpeed = 0;       // speed at which enemy patrols their spawnpoint
    float currentPatrolTime = 0; // current time patrolling 
    float maxPatrolTime = 0;     // max time you patrol in that direction'
    float timeSinceForcedRotation = 9999;
};

struct FlyingBehavior : Behavior {};

struct WalkingBehavior : Behavior {
    bool stupid = true;
    bool jumpRequest = false;
};

struct RangedBehavior : Behavior {
    bool lobbing = false;
    i32 attackPower = 4;
    float attackCooldown = 2000.f;
    float elapsedTime = 0.f;
};

struct MeleeBehavior : Behavior {
    i32 attackPower = 8;
    bool requestingAttack = false;
    float attackCooldown = 500;
    float elapsedTime = 0;
};

struct Weapon
{
    float damage = 1.f;
    float throwDamage = 0.f;
    bool ranged = false;
    float cooldown = 0.f;
};

struct Item
{
    // When player is holding an item, we don't want to have it colliding with the floor/ceiling etc
    // When item is on the floor, we want it to be colliding with the floor
    bool collidableWithEnvironment = true;
    bool grounded = true;
    bool friction = true;
    bool pickable = true;
};

struct ShopItem
{
    u8 mutationIndex;
};

struct ActiveShopItem 
{
    std::vector<Entity> linkedEntity = {};
};

struct PlayerProjectile
{
    float elapsed_time = 0.f;
    float minTravelTime = 0.4f; // projectile keeps moving regardless of friction for at least this duration
    bool bHitWall = false;
};

struct ActivePlayerProjectile {
    float damage = 0.f;
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
    vec2 drag = { 0.f, 0.f };                   // unsigned
    vec2 terminalVelocity = { 9999.f, 9999.f }; // unsigned
    bool facingRight = true;
};

struct CollisionComponent
{
    vec2 collider_position = { 0.f, 0.f };
    // Collision
    vec2 collision_pos = { 0, 0 }; // Collision box x,y size in the positive direction from the center
    vec2 collision_neg = { 0, 0 }; // Collision box x,y size in the negative direction from the center
};

struct VisionComponent
{
    float sightRadius = 0.f;
    bool hasAggro = false;
};

struct Animation // NOT A COMPONENT
{
    u16 num_frames = 1;
    u16 start_frame = 0;
    float animation_duration = 0.f;
    bool replay = true;
    bool played = false;
    bool go_back_to_first = true;
};

struct SpriteComponent
{
    // Don't change the order please - it's ordered to reduce struct padding
    shortvec2           dimensions  = { 0, 0 }; // in pixels
    i8                  layer       = 0;        // render layer. higher layer is drawn on top of lower layers. -128 to 127
    EFFECT_ASSET_ID     shaderId    = EFFECT_ASSET_ID::SPRITE; // ID for the shader to use
    TEXTURE_ASSET_ID    texId       = TEXTURE_ASSET_ID::BOX; // ID for the texture to use

    // SPRITESHEET / ANIMATION DATA
    bool sprite_sheet = false;
    bool reverse = false;
    bool faceRight = true; // if sprite facing right, true

    u16 sheetSizeX = 16;
    u16 sheetSizeY = 16;

    u8 selected_animation = 0;
    u8 current_frame = 0;
    float elapsed_time = 0.f;

    std::vector<Animation> animations = {};

    void SetStartFrame(u16 frame)
    {
        animations[selected_animation].start_frame = frame;
    }

    u16 GetStartFrame() const
    {
        return animations[selected_animation].start_frame;
    }
};

struct DeathTimer 
{
    float elapsed_ms = 550.f;
};

//For entities that can hold items
struct HolderComponent
{
    int current_item = -1;
    Entity near_weapon = Entity();
    Entity held_weapon = Entity();

    std::vector<Entity> carried_items;

    bool want_to_pick_up = false;
    bool want_to_drop = false;
    bool want_to_throw = false;
    bool want_to_shoot = false;
    bool want_to_melee = false;
    bool want_to_cycle_left = false;
    bool want_to_cycle_right = false;
    bool want_to_shoot_up = false;
    bool want_to_shoot_down = false;
};

struct CollisionEvent
{
    // Stucture to store collision information
    vec2 collision_overlap = {0, 0};
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	CollisionEvent(Entity& other) { this->other = other; };
};

struct Mutation 
{
    std::string name;
    std::string description;
    SpriteComponent sprite;
    std::function<void(Entity)> effect; // mutation effect function
    bool bTriggered = false;
};

struct ActiveMutationsComponent 
{
    std::vector<Mutation> mutations; // the active mutations of a given entity
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = false;
	bool in_freeze_mode = false;
};
extern Debug debugging;

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

struct HealthBar
{
	float health = 100.f;
    float maxHealth = 100.f;
    bool bInvincible = false;

    void Heal(float amount)
    {
        health = min(health + amount, maxHealth);
    }

    float TakeDamage(float base, float variance = 0.f)
    {
        if(bInvincible)
        {
            return 0.f;
        }

        float low = base - variance;
        float high = base + variance;
        float actualDamage = low + static_cast<float>(rand())/(static_cast<float>(RAND_MAX/(high-low)));
        health -= actualDamage;
        return actualDamage;
    }
};

struct GoldBar
{
    float coins = 50.f;
};

struct ProximityTextComponent
{
    vec2 triggerPosition = {0.f,0.f};
    float triggerRadius = 32.f;
    vec2 textPosition = {0.f,0.f};
    u32 textSize = 16;
    std::string text = "DEFAULT TEXT";

    bool bTyped = false;
    float secondsBetweenTypedCharacters = 0.05f;

    bool __bCurrentlyTriggered = false;
    float __typingTimer = 0.f;
    std::string __currentTypedTextBuffer = "";
};

struct LightSource
{

};

enum GAMETAGS : u8
{
    TAG_DEFAULT,
    TAG_PLAYER,
    TAG_LEVELENDPOINT,
    TAG_LADDER,
    TAG_SPIKE,
    TAG_PLAYERBLOCKABLE,
    TAG_PLAYERMELEEATTACK,
    TAG_SWORD,
    TAG_BOW,
    TAG_ENEMYMELEEATTACK,
    TAG_BOSSMELEEATTACK,
    TAG_WALKINGBOMB
};
