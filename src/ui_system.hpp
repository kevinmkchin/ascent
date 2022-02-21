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

private:
    RenderSystem* renderer;
    WorldSystem* world;
    PlayerSystem* playerSystem;
};