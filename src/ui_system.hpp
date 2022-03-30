#pragma once

#include "common.hpp"

class RenderSystem;
class WorldSystem;
class PlayerSystem;
struct vtxt_font;
struct TextureHandle;

void LoadFont(vtxt_font* font_handle, TextureHandle* font_atlas, const char* font_path, u8 font_size, bool useNearest = false);

class UISystem
{
public:
    UISystem();

    void Init(RenderSystem* render_sys_arg, WorldSystem* world_sys_arg, PlayerSystem* player_sys_arg);

    void Step(float deltaTime);

    float* GlobalPauseForSeconds = nullptr;

    u8 cachedGameStage;

private:

    void UpdateHealthBarUI(float dt);

    void UpdateExpUI(float dt);

    void UpdateTextUI(float dt);
    
    void UpdateLevelUpUI(float dt);

    void UpdateShopUI(float dt);

    RenderSystem* renderer;
    WorldSystem* world;
    PlayerSystem* playerSystem;
};