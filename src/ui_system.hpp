#pragma once

class RenderSystem;
class WorldSystem;
class PlayerSystem;

class UISystem
{
public:
    UISystem();

    void Init(RenderSystem* render_sys_arg, WorldSystem* world_sys_arg, PlayerSystem* player_sys_arg);

    void Step(float deltaTime);

    float* GlobalPauseForSeconds = nullptr;

private:

    void UpdateExpUI(float dt);

    void UpdateTextUI(float dt);
    
    void UpdateLevelUpUI(float dt);

    void UpdateShopUI(float dt);

    RenderSystem* renderer;
    WorldSystem* world;
    PlayerSystem* playerSystem;
};