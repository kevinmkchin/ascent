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
INTERNAL vtxt_font font_c64;
INTERNAL TextureHandle texture_c64;
INTERNAL vtxt_font font_medusa_gothic;
INTERNAL TextureHandle texture_medusa_gothic;


INTERNAL void LoadFont(vtxt_font* font_handle, TextureHandle* font_atlas, const char* font_path, u8 font_size, bool useNearest = false)
{
    BinaryFileHandle fontfile;
    ReadFileBinary(fontfile, font_path);
    assert(fontfile.memory);
    vtxt_init_font(font_handle, (u8*) fontfile.memory, font_size);
    FreeFileBinary(fontfile);
    CreateTextureFromBitmap(*font_atlas, font_handle->font_atlas.pixels, font_handle->font_atlas.width, 
        font_handle->font_atlas.height, GL_RED, GL_RED, (useNearest ? GL_NEAREST : GL_LINEAR));
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

    LoadFont(&font_c64, &texture_c64, font_path("c64.ttf").c_str(), 32, true);
    LoadFont(&font_medusa_gothic, &texture_medusa_gothic, font_path("medusa-gothic.otf").c_str(), TEXT_SIZE);
}

#pragma warning(push)
#pragma warning(disable : 4996)
void UISystem::Step(float deltaTime)
{
    vtxt_clear_buffer();

    switch(world->GetCurrentMode())
    {
        case MODE_MAINMENU:
        {
            vtxt_move_cursor(400, 440);
            vtxt_append_line("ASCENT", &font_c64, 72);
            vtxt_move_cursor(400, 600);
            vtxt_append_line("Press ENTER to play", &font_c64, 48);
        }break;
        case MODE_INGAME:
        {
            Entity playerEntity = registry.players.entities[0];
            TransformComponent& playerTransform = registry.transforms.get(playerEntity);
            MotionComponent& playerMotion = registry.motions.get(playerEntity);
            CollisionComponent& playerCollider = registry.colliders.get(playerEntity);
            HealthBar& playerHealth = registry.healthBar.get(playerEntity);

            char textBuffer[128];
            sprintf(textBuffer, "Health: %d", (int) playerHealth.health);
            vtxt_move_cursor(20, 60);
            vtxt_append_line(textBuffer, &font_c64, 40);
            sprintf(textBuffer, "Gold: %d", (int) 99);
            vtxt_move_cursor(20, 112);
            vtxt_append_line(textBuffer, &font_c64, 40);
        }break;
    }

    vtxt_vertex_buffer vb = vtxt_grab_buffer();
    renderer->textLayer1FontAtlas = texture_c64;
    renderer->textLayer1Colour = vec4(1.f,1.f,1.f,1.0f);
    RebindMeshBufferObjects(renderer->textLayer1VAO, vb.vertex_buffer, vb.index_buffer, vb.vertices_array_count, vb.indices_array_count);

    // chapter change text
    LOCAL_PERSIST GAMELEVELENUM cachedGameStage = GAME_NOT_STARTED;
    LOCAL_PERSIST bool showChapterText = false;
    LOCAL_PERSIST float chapterTextAlpha = 0.f;
    if(world->GetCurrentStage() != cachedGameStage)
    {
        cachedGameStage = world->GetCurrentStage();
        showChapterText = true;
        chapterTextAlpha = 1.8f;
    }
    if(showChapterText)
    {
        chapterTextAlpha -= 0.5f * deltaTime;
        if(chapterTextAlpha < 0.f)
        {
            showChapterText = false;
        }

        vtxt_clear_buffer();

        switch(cachedGameStage)
        {
            case CHAPTER_ONE_STAGE_ONE:
            {
                vtxt_move_cursor(150,400);
                vtxt_append_line("Chapter One", &font_medusa_gothic, 180);
                vtxt_move_cursor(420,530);
                vtxt_append_line("Stage One", &font_medusa_gothic, 110);
            }break;
            case CHAPTER_ONE_STAGE_TWO:
            {
                vtxt_move_cursor(150,400);
                vtxt_append_line("Chapter One", &font_medusa_gothic, 180);
                vtxt_move_cursor(420,530);
                vtxt_append_line("Stage Two", &font_medusa_gothic, 110);
            }break;
            // case CHAPTER_ONE_STAGE_THREE:
            // {
            //     vtxt_move_cursor(150,400);
            //     vtxt_append_line("Chapter One", &font_medusa_gothic, 180);
            //     vtxt_move_cursor(420,530);
            //     vtxt_append_line("Stage Three", &font_medusa_gothic, 110);
            // }break;
            // case CHAPTER_TWO_STAGE_ONE:
            // {
            //     vtxt_move_cursor(150,400);
            //     vtxt_append_line("Chapter Two", &font_medusa_gothic, 180);
            //     vtxt_move_cursor(420,530);
            //     vtxt_append_line("Stage One", &font_medusa_gothic, 110);
            // }break;
        }

        vb = vtxt_grab_buffer();
        renderer->textLayer2FontAtlas = texture_medusa_gothic;
        renderer->textLayer2Colour = vec4(1.f,1.f,1.f,chapterTextAlpha);
        RebindMeshBufferObjects(renderer->textLayer2VAO, vb.vertex_buffer, vb.index_buffer, vb.vertices_array_count, vb.indices_array_count);
    }
}
#pragma warning(pop)
