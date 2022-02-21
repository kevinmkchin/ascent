#include "ui_system.hpp"
#include "common.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "player_system.hpp"
#include "file_system.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#define VERTEXT_IMPLEMENTATION
#include "vertext.h"

#define TEXT_SIZE 64
INTERNAL vtxt_font font_VGA437;
INTERNAL TextureHandle texture_VGA437;

INTERNAL void LoadFont(vtxt_font* font_handle, TextureHandle* font_atlas, const char* font_path, u8 font_size)
{
    BinaryFileHandle fontfile;
    ReadFileBinary(fontfile, font_path);
    assert(fontfile.memory);
    vtxt_init_font(font_handle, (u8*) fontfile.memory, font_size);
    FreeFileBinary(fontfile);
    CreateTextureFromBitmap(*font_atlas, font_handle->font_atlas.pixels, font_handle->font_atlas.width, 
        font_handle->font_atlas.height, GL_RED, GL_RED);
    free(font_handle->font_atlas.pixels);
}

UISystem::UISystem()
{

}

void UISystem::Init(RenderSystem* render_sys_arg, WorldSystem* world_sys_arg, PlayerSystem* player_sys_arg)
{
    renderer = render_sys_arg;
    world = world_sys_arg;
    playerSystem = player_sys_arg;

    vtxt_setflags(VTXT_CREATE_INDEX_BUFFER|VTXT_USE_CLIPSPACE_COORDS);
    vtxt_backbuffersize(UI_LAYER_RESOLUTION_WIDTH, UI_LAYER_RESOLUTION_HEIGHT);

    LoadFont(&font_VGA437, &texture_VGA437, font_path("dos_vga_437.ttf").c_str(), TEXT_SIZE);
}

#pragma warning(push)
#pragma warning(disable : 4996)
void UISystem::Step(float deltaTime)
{
    // CLEAR
    vtxt_clear_buffer();

    // WRITE TEXT
    switch(world->GetCurrentMode())
    {
        case MODE_MAINMENU:
        {
            vtxt_move_cursor(420, 440);
            vtxt_append_line("ASCENT", &font_VGA437, 128);
            vtxt_move_cursor(420, 600);
            vtxt_append_line("Press ENTER to play", &font_VGA437, TEXT_SIZE);
        }break;
        case MODE_INGAME:
        {
            Entity playerEntity = registry.players.entities[0];
            TransformComponent& playerTransform = registry.transforms.get(playerEntity);
            MotionComponent& playerMotion = registry.motions.get(playerEntity);
            CollisionComponent& playerCollider = registry.colliders.get(playerEntity);
            HealthBar& playerHealth = registry.healthBar.get(playerEntity);

            vtxt_move_cursor(20, 60);
            char textBuffer[128];
            sprintf(textBuffer, "Health: %d", (int) playerHealth.health);
            vtxt_append_line(textBuffer, &font_VGA437, 48);
            vtxt_move_cursor(600, 64);
            vtxt_append_line("Wow! Cool UI Text!", &font_VGA437, 64);
            vtxt_move_cursor(60, 1000);
            vtxt_append_line("Amazing. UwU", &font_VGA437, 64);
        }break;
    }

    // DRAW
    vtxt_vertex_buffer vb = vtxt_grab_buffer();
    renderer->textLayer1FontAtlas = texture_VGA437;
    renderer->textLayer1Colour = vec3(1.f,1.f,1.f);
    RebindMeshBufferObjects(renderer->textLayer1VAO, vb.vertex_buffer, vb.index_buffer, vb.vertices_array_count, vb.indices_array_count);
}
#pragma warning(pop)
