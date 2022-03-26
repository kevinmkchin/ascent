#include "ui_system.hpp"
#include "common.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "player_system.hpp"
#include "file_system.hpp"
#include "input.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#define VERTEXT_IMPLEMENTATION
#include "vertext.h"

#define TEXT_SIZE 64
INTERNAL vtxt_font font_c64;
INTERNAL TextureHandle texture_c64;
INTERNAL vtxt_font font_medusa_gothic;
INTERNAL TextureHandle texture_medusa_gothic;


void LoadFont(vtxt_font* font_handle, TextureHandle* font_atlas, const char* font_path, u8 font_size, bool useNearest)
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
    cachedGameStage = GAME_NOT_STARTED;
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

void UISystem::UpdateExpUI(float dt)
{
    if(registry.players.size() > 0)
    {
        Player playerComponent = registry.players.components[0];

        float currentExp = playerComponent.experience;
        float lowerBound = 0.f;
        float upperBound = 9999.f;
        for(int i = 1; i < ARRAY_COUNT(PLAYER_EXP_THRESHOLDS_ARRAY); ++i)
        {
            float il = PLAYER_EXP_THRESHOLDS_ARRAY[i-1];
            float iu = PLAYER_EXP_THRESHOLDS_ARRAY[i];
            if(currentExp < iu)
            {
                lowerBound = il;
                upperBound = iu;
                break;
            }
        }

        renderer->expProgressNormalized = ((currentExp - lowerBound) / (upperBound - lowerBound));
    }
}

#pragma warning(push)
#pragma warning(disable : 4996)
void UISystem::UpdateTextUI(float dt)
{
    vtxt_clear_buffer();

    switch(world->GetCurrentMode())
    {
        case MODE_MAINMENU:
        {
            vtxt_move_cursor(350, 340);
            vtxt_append_line("ASCENT", &font_c64, 72);
            vtxt_move_cursor(350, 500);
            vtxt_append_line("PLAY (ENTER)", &font_c64, 48);
            vtxt_move_cursor(350, 660);
            vtxt_append_line("EXIT (Q)", &font_c64, 48);
            vtxt_move_cursor(350, 820);
            if (world->GetCurrentDifficulty() == DIFFICULTY_EASY) { 
                vtxt_append_line("SWAP DIFFICULTY [STANDARD] (R)", &font_c64, 36); 
            }
            else {
                vtxt_append_line("SWAP DIFFICULTY [HARD] (R)", &font_c64, 36);
            }
        }break;
        case MODE_INGAME:
        {
            Entity playerEntity = registry.players.entities[0];
            TransformComponent& playerTransform = registry.transforms.get(playerEntity);
            MotionComponent& playerMotion = registry.motions.get(playerEntity);
            CollisionComponent& playerCollider = registry.colliders.get(playerEntity);
            HealthBar& playerHealth = registry.healthBar.get(playerEntity);
            GoldBar& playerGold = registry.goldBar.get(playerEntity);

            char textBuffer[128];

            int displayHealth = (playerHealth.health > 0.f && playerHealth.health < 1.f) ? 1 : (int) playerHealth.health;
            sprintf(textBuffer, "HP: %d/%d", displayHealth, (int) playerHealth.maxHealth);
            vtxt_move_cursor(26, 66);
            vtxt_append_line(textBuffer, &font_c64, 40);

            sprintf(textBuffer, "Gold: %d", (int)playerGold.coins);
            vtxt_move_cursor(26, 130);
            vtxt_append_line(textBuffer, &font_c64, 40);

            sprintf(textBuffer, "Lvl %d", (int) 1);
            if(registry.players.size() > 0)
            {      
                Player playerComponent = registry.players.components[0];
                sprintf(textBuffer, "Lvl %d", playerComponent.level);
            }
            vtxt_move_cursor(900, UI_LAYER_RESOLUTION_HEIGHT - 8);
            vtxt_append_line(textBuffer, &font_c64, 24);

            if(world->gamePaused)
            {
                vtxt_move_cursor(860, 556);
                vtxt_append_line("PAUSED", &font_c64, 32);
            }

            if(registry.players.size() > 0)
            {      
                Player playerComponent = registry.players.components[0];
                if(playerComponent.bDead)
                {
                    vtxt_move_cursor(700, 580);
                    vtxt_append_line("GAME OVER", &font_c64, 64);
                }
            }

        }break;
    }

    vtxt_vertex_buffer vb = vtxt_grab_buffer();
    renderer->textLayer1FontAtlas = texture_c64;
    renderer->textLayer1Colour = vec4(1.f,1.f,1.f,1.0f);
    RebindMeshBufferObjects(renderer->textLayer1VAO, vb.vertex_buffer, vb.index_buffer, vb.vertices_array_count, vb.indices_array_count);

    LOCAL_PERSIST float showTutorialTimer = 0.f;

    // chapter change text
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
        if(!world->gamePaused)
        {
            chapterTextAlpha -= 0.5f * dt;
        }

        if(chapterTextAlpha < 0.f)
        {
            showChapterText = false;

            if(cachedGameStage == CHAPTER_ONE_STAGE_ONE)
            {
                *GlobalPauseForSeconds = 9999.f;
                showTutorialTimer = 9999.f;
                world->darkenGameFrame = true;
            }
        }

        vtxt_clear_buffer();

        if(!world->gamePaused)
        {
            switch(cachedGameStage)
            {
                case CHAPTER_ONE_STAGE_ONE:
                {
                    vtxt_move_cursor(390,500);
                    vtxt_append_line("Chapter One", &font_medusa_gothic, 180);
                    vtxt_move_cursor(650,630);
                    vtxt_append_line("Stage One", &font_medusa_gothic, 110);
                }break;
                case CHAPTER_ONE_STAGE_TWO:
                {
                    vtxt_move_cursor(390,500);
                    vtxt_append_line("Chapter One", &font_medusa_gothic, 180);
                    vtxt_move_cursor(650,630);
                    vtxt_append_line("Stage Two", &font_medusa_gothic, 110);
                }break;
                case CHAPTER_ONE_STAGE_THREE:
                {
                    vtxt_move_cursor(390,500);
                    vtxt_append_line("Chapter One", &font_medusa_gothic, 180);
                    vtxt_move_cursor(650,630);
                    vtxt_append_line("Stage Three", &font_medusa_gothic, 110);
                }break;
                // case CHAPTER_TWO_STAGE_ONE:
                // {
                //     vtxt_move_cursor(390,500);
                //     vtxt_append_line("Chapter Two", &font_medusa_gothic, 180);
                //     vtxt_move_cursor(650,630);
                //     vtxt_append_line("Stage One", &font_medusa_gothic, 110);
                // }break;
            }
        }

        vb = vtxt_grab_buffer();
        renderer->textLayer2FontAtlas = texture_medusa_gothic;
        renderer->textLayer2Colour = vec4(1.f,1.f,1.f,chapterTextAlpha);
        RebindMeshBufferObjects(renderer->textLayer2VAO, vb.vertex_buffer, vb.index_buffer, vb.vertices_array_count, vb.indices_array_count);
    }

    if(showTutorialTimer > 0.f && !world->gamePaused)
    {
        vtxt_clear_buffer();

        showTutorialTimer -= dt;

        if(Input::HasKeyBeenPressed(SDL_SCANCODE_J) || Input::GetGamepad(0).HasBeenPressed(GAMEPAD_A))
        {
            showTutorialTimer = -1.f;
            *GlobalPauseForSeconds = -1.f;
            world->darkenGameFrame = false;
        }

        if(Input::GetGamepad(0).isConnected)
        {
            vtxt_move_cursor(260, 190);
            vtxt_append_line("D-Pad or Left Thumbstick to move.", &font_c64, 40);
            vtxt_move_cursor(260, 260);
            vtxt_append_line("A to jump", &font_c64, 40);
            vtxt_move_cursor(260, 330);
            vtxt_append_line("X to attack", &font_c64, 40);
            vtxt_move_cursor(260, 400);
            vtxt_append_line("B to pick up item", &font_c64, 40);
            vtxt_move_cursor(260, 450);
            vtxt_append_line("B while holding item to throw item", &font_c64, 40);
            vtxt_move_cursor(260, 500);
            vtxt_append_line("B + down to drop item", &font_c64, 40);
            vtxt_move_cursor(320, 800);
            vtxt_append_line("Press A to continue...", &font_c64, 40);
        }
        else
        {
            vtxt_move_cursor(260, 190);
            vtxt_append_line("WASD to move.", &font_c64, 40);
            vtxt_move_cursor(260, 260);
            vtxt_append_line("J to jump", &font_c64, 40);
            vtxt_move_cursor(260, 330);
            vtxt_append_line("K to attack", &font_c64, 40);
            vtxt_move_cursor(260, 400);
            vtxt_append_line("L to pick up item", &font_c64, 40);
            vtxt_move_cursor(260, 450);
            vtxt_append_line("L while holding item to throw item", &font_c64, 40);
            vtxt_move_cursor(260, 500);
            vtxt_append_line("L + down to drop item", &font_c64, 40);
            vtxt_move_cursor(320, 800);
            vtxt_append_line("Press J to continue...", &font_c64, 40);
        }

        vtxt_vertex_buffer vb = vtxt_grab_buffer();
        renderer->textLayer1FontAtlas = texture_c64;
        renderer->textLayer1Colour = vec4(1.f,1.f,1.f,1.0f);
        RebindMeshBufferObjects(renderer->textLayer1VAO, vb.vertex_buffer, vb.index_buffer, vb.vertices_array_count, vb.indices_array_count);
    }
}

void UISystem::UpdateLevelUpUI(float dt)
{
    vtxt_clear_buffer();

    renderer->showMutationSelect = false;

    switch(world->GetCurrentMode())
    {
        case MODE_INGAME:
        {
            Entity playerEntity = registry.players.entities[0];

            LOCAL_PERSIST float levelUpTextTimer = 0.f;
            LOCAL_PERSIST bool pickThreeMutations = false;
            LOCAL_PERSIST Mutation mutationOptions[3];
            if(playerSystem->bLeveledUpLastFrame)
            {
                levelUpTextTimer = 100000.0f;
                *GlobalPauseForSeconds = 100000.0f;
                pickThreeMutations = true;
            }

            if(levelUpTextTimer > 0.f)
            {
                if(!world->gamePaused)
                {
                    levelUpTextTimer -= dt;
                }

                if(levelUpTextTimer > 99999.f)
                {
                    vtxt_move_cursor(670, 580);
                    vtxt_append_line("Level Up!", &font_c64, 80);
                }
                else
                {
                    if(pickThreeMutations)
                    {
                        std::vector<Mutation> mutations = world->allPossibleMutations;
                        // TODO(Kevin): remove mutations that player already has
                        int mut1;
                        int mut2;
                        int mut3;
                        PickThreeRandomInts(&mut1, &mut2, &mut3, (int)mutations.size());
                        mutationOptions[0] = mutations[mut1];
                        mutationOptions[1] = mutations[mut2];
                        mutationOptions[2] = mutations[mut3];
                        pickThreeMutations = false;
                    }

                    // mutationOptions
                    vtxt_move_cursor(180, 350);
                    vtxt_append_line(mutationOptions[0].name.c_str(), &font_c64, 28);
                    vtxt_move_cursor(754, 350);
                    vtxt_append_line(mutationOptions[1].name.c_str(), &font_c64, 28);
                    vtxt_move_cursor(1333, 350);
                    vtxt_append_line(mutationOptions[2].name.c_str(), &font_c64, 28);

                    // 19 char wide
                    std::string mut1desc = mutationOptions[0].description;
                    std::string mut2desc = mutationOptions[1].description;
                    std::string mut3desc = mutationOptions[2].description;
                    int descCursorY = 500;
                    while(mut1desc.length() > 0)
                    {
                        std::string toPrint = mut1desc.substr(0, 24);
                        mut1desc = mut1desc.erase(0, 24);
                        vtxt_move_cursor(180, descCursorY);
                        descCursorY += 25;
                        vtxt_append_line(toPrint.c_str(), &font_c64, 20);
                    }

                    descCursorY = 500;
                    while(mut2desc.length() > 0)
                    {
                        std::string toPrint = mut2desc.substr(0, 24);
                        mut2desc = mut2desc.erase(0, 24);
                        vtxt_move_cursor(754, descCursorY);
                        descCursorY += 25;
                        vtxt_append_line(toPrint.c_str(), &font_c64, 20);
                    }

                    descCursorY = 500;
                    while(mut3desc.length() > 0)
                    {
                        std::string toPrint = mut3desc.substr(0, 24);
                        mut3desc = mut3desc.erase(0, 24);
                        vtxt_move_cursor(1333, descCursorY);
                        descCursorY += 25;
                        vtxt_append_line(toPrint.c_str(), &font_c64, 20);
                    }

                    vtxt_move_cursor(574, 900);
                    if(Input::GetGamepad(0).isConnected)
                    {
                        vtxt_append_line("Press A to select mutation...", &font_c64, 32);
                    }
                    else
                    {
                        vtxt_append_line("Press J to select mutation...", &font_c64, 32);
                    }

                    if(Input::GameLeftHasBeenPressed())
                    {
                        --(renderer->mutationSelectionIndex);
                        if(Mix_PlayChannel(-1, world->blip_select_sound, 0) == -1) 
                        {
                            printf("Mix_PlayChannel: %s\n",Mix_GetError());
                        }
                    }
                    if(Input::GameRightHasBeenPressed())
                    {
                        ++(renderer->mutationSelectionIndex);
                        if(Mix_PlayChannel(-1, world->blip_select_sound, 0) == -1) 
                        {
                            printf("Mix_PlayChannel: %s\n",Mix_GetError());
                        }
                    }
                    renderer->mutationSelectionIndex = (renderer->mutationSelectionIndex + 3) % 3;
                    renderer->showMutationSelect = true;

                    if(Input::GameJumpHasBeenPressed())
                    {
                        Mutation mutationToAdd = mutationOptions[renderer->mutationSelectionIndex];
                        ActiveMutationsComponent& playerActiveMutations = registry.mutations.get(playerEntity);
                        playerActiveMutations.mutations.push_back(mutationToAdd);

                        renderer->mutationSelectionIndex = 1;
                        levelUpTextTimer = -1.f;
                        *GlobalPauseForSeconds = 0.0f;
                    }
                }
            }

        }break;
        default:
        {}break;
    }

    vtxt_vertex_buffer vb = vtxt_grab_buffer();
    renderer->textLayer3FontAtlas = texture_c64;
    renderer->textLayer3Colour = vec4(1.f,1.f,1.f,1.0f);
    RebindMeshBufferObjects(renderer->textLayer3VAO, vb.vertex_buffer, vb.index_buffer, vb.vertices_array_count, vb.indices_array_count);  
}

#pragma warning(pop)

void UISystem::Step(float deltaTime)
{
    // Check pause
    if(world->GetCurrentMode() == MODE_INGAME && Input::GamePauseHasBeenPressed())
    {
        if(world->gamePaused)
        {
            // unpause
            world->gamePaused = false;
            world->darkenGameFrame = false;
        }
        else
        {
            // pause
            world->gamePaused = true;
            world->darkenGameFrame = true;
        }
    }

    UpdateExpUI(deltaTime);
    UpdateTextUI(deltaTime);
    UpdateLevelUpUI(deltaTime);
}
