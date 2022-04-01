#include <string>
#include <vector>
#include <SDL.h>
#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>
#include "glm/gtc/matrix_transform.hpp"

#include "vertext.h"
#include "console.hpp"
#include "common.hpp"
#include "components.hpp"
#include "render_system.hpp"

/**

    QUAKE-STYLE IN-GAME CONSOLE IMPLEMENTATION

    There are two parts to the in-game console:

    1.  console.h/cpp:
        console.h is the interface that the rest of the game uses to communicate with console.cpp,
        console.cpp handles the console visuals, inputs, outputs, and logic related to console messages

    2.  noclip.h:
        noclip.h is the backend of the console and handles executing commands

*/

INTERNAL noclip::console console_backend;
noclip::console& get_console()
{
    return console_backend;
}

#define CONSOLE_MAX_PRINT_MSGS 8096
#define CONSOLE_SCROLL_SPEED 4.0f
#define CONSOLE_COLS_MAX 124        // char columns in line
#define CONSOLE_ROWS_MAX 27         // we can store more messages than this, but this is just rows that are being displayed
enum console_state_t
{
    CONSOLE_HIDING,
    CONSOLE_HIDDEN,
    CONSOLE_SHOWING,
    CONSOLE_SHOWN
};
INTERNAL GLuint console_background_vao_id = 0;
INTERNAL GLuint console_background_vbo_id = 0;
INTERNAL GLfloat console_background_vertex_buffer[] = {
    // INITIAL VALUES DON'T MATTER - OVERWRITTEN IN console_initalize 2022-03-25
    0.f,    1.f,        0.f, 0.f,
    0.f,    400.f,      0.f, 1.f,
    1280.f, 400.f,      1.f, 1.f,
    1280.f, 1.f,        1.f, 0.f,
    0.f,    1.f,        0.f, 0.f,
    1280.f, 400.f,      1.f, 1.f
};
INTERNAL GLuint console_line_vao_id = 0;
INTERNAL GLuint console_line_vbo_id = 0;
INTERNAL GLfloat console_line_vertex_buffer[] = {
    0.f, 400.f,
    1280.f, 400.f
};

INTERNAL bool               console_b_initialized = false;
INTERNAL console_state_t    console_state = CONSOLE_HIDDEN;
INTERNAL float              console_y = -1.f;

INTERNAL float    CONSOLE_HEIGHT = 0.1f;//400.f;
INTERNAL u8       CONSOLE_TEXT_SIZE = 40;
INTERNAL u8       CONSOLE_TEXT_PADDING_BOTTOM = 4;
INTERNAL u16      CONSOLE_INPUT_DRAW_X = 4;
INTERNAL u16      CONSOLE_INPUT_DRAW_Y = (u16) ((((1.f+CONSOLE_HEIGHT)/2.f)*1080) - (float) CONSOLE_TEXT_PADDING_BOTTOM);

// Input character buffer
INTERNAL char        console_input_buffer[CONSOLE_COLS_MAX];
INTERNAL bool        console_b_input_buffer_dirty = false;
INTERNAL u8       console_input_cursor = 0;
INTERNAL u8       console_input_buffer_count = 0;

// Hidden character buffer
INTERNAL char        console_messages[CONSOLE_MAX_PRINT_MSGS] = {};
INTERNAL u16      console_messages_read_cursor = 0;
INTERNAL u16      console_messages_write_cursor = 0;
INTERNAL bool        console_b_messages_dirty = false;

// Text visuals
INTERNAL vtxt_font*     console_font_handle;
INTERNAL TextureHandle  console_font_atlas;
// Input text & Messages VAOs
INTERNAL MeshHandle        console_inputtext_vao; // console_inputtext_vao gets added to console_messages_vaos if user "returns" command
INTERNAL MeshHandle        console_messages_vaos[CONSOLE_ROWS_MAX] = {}; // one vao is one line

INTERNAL RenderSystem* renderSystem;
INTERNAL vec2 buffer_dimensions;

// TODO buffer to hold previous commands (max 20 commands)

void console_initialize(vtxt_font* in_console_font_handle, TextureHandle in_console_font_atlas, RenderSystem* renderer)
{
    console_font_handle = in_console_font_handle;
    console_font_atlas = in_console_font_atlas;
    renderSystem = renderer;

    vtxt_setflags(VTXT_CREATE_INDEX_BUFFER|VTXT_USE_CLIPSPACE_COORDS);
    vtxt_backbuffersize(UI_LAYER_RESOLUTION_WIDTH, UI_LAYER_RESOLUTION_HEIGHT);

    // INIT TEXT mesh_t OBJECTS
    vtxt_clear_buffer();
    vtxt_move_cursor(CONSOLE_INPUT_DRAW_X, CONSOLE_INPUT_DRAW_Y);
    vtxt_append_glyph('>', console_font_handle, CONSOLE_TEXT_SIZE);
    vtxt_vertex_buffer vb = vtxt_grab_buffer();
    CreateMeshVertexArray(console_inputtext_vao, vb.vertex_buffer, vb.index_buffer,
                           vb.vertices_array_count, vb.indices_array_count,
                           2, 2, 0, GL_DYNAMIC_DRAW);
    // INIT MESSAGES mesh_t OBJECTS
    for(int i = 0; i < CONSOLE_ROWS_MAX; ++i)
    {
        CreateMeshVertexArray(console_messages_vaos[i], nullptr, nullptr,
                               0, 0,
                               2, 2, 0, GL_DYNAMIC_DRAW);
    }

    // todo update console vertex buffer on window size change
    // INIT CONSOLE GUI
    buffer_dimensions = vec2((float) UI_LAYER_RESOLUTION_WIDTH, (float) UI_LAYER_RESOLUTION_HEIGHT);
    float halfWidth = 1.f;//buffer_dimensions.x / 2.f;
    console_background_vertex_buffer[0] = -halfWidth;
    console_background_vertex_buffer[4] = -halfWidth;
    console_background_vertex_buffer[16] = -halfWidth;
    console_background_vertex_buffer[8] = halfWidth;
    console_background_vertex_buffer[12] = halfWidth;
    console_background_vertex_buffer[20] = halfWidth;
    console_background_vertex_buffer[5] = -CONSOLE_HEIGHT;
    console_background_vertex_buffer[9] = -CONSOLE_HEIGHT;
    console_background_vertex_buffer[21] = -CONSOLE_HEIGHT;
    glGenVertexArrays(1, &console_background_vao_id);
    glBindVertexArray(console_background_vao_id);
        glGenBuffers(1, &console_background_vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, console_background_vbo_id);
            glBufferData(GL_ARRAY_BUFFER, sizeof(console_background_vertex_buffer), console_background_vertex_buffer, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
            glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    console_line_vertex_buffer[0] = -halfWidth;
    console_line_vertex_buffer[1] = -CONSOLE_HEIGHT + (((float) CONSOLE_TEXT_SIZE + CONSOLE_TEXT_PADDING_BOTTOM) / (buffer_dimensions.y*0.5f));
    console_line_vertex_buffer[2] = halfWidth;
    console_line_vertex_buffer[3] = -CONSOLE_HEIGHT + (((float) CONSOLE_TEXT_SIZE + CONSOLE_TEXT_PADDING_BOTTOM) / (buffer_dimensions.y*0.5f));
    glGenVertexArrays(1, &console_line_vao_id);
    glBindVertexArray(console_line_vao_id);
        glGenBuffers(1, &console_line_vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, console_line_vbo_id);
            glBufferData(GL_ARRAY_BUFFER, sizeof(console_line_vertex_buffer), console_line_vertex_buffer, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    console_b_initialized = true;
    console_print("Console initialized.\n");
}

/** logs the message into the messages buffer */
void console_print(const char* message)
{

#if INTERNAL_BUILD & SLOW_BUILD
    printf(message);
#endif

    // commands get con_printed when returned
    int i = 0;
    while(*(message + i) != '\0')
    {
        console_messages[console_messages_write_cursor] = *(message + i);
        ++console_messages_write_cursor;
        if(console_messages_write_cursor >= CONSOLE_MAX_PRINT_MSGS)
        {
            console_messages_write_cursor = 0;
        }
        ++i;
    }
    console_messages_read_cursor = console_messages_write_cursor;
    console_b_messages_dirty = true;
}

void console_printf(const char* fmt, ...)
{
    va_list argptr;

    char message[1024];
    va_start(argptr, fmt);
    stbsp_vsprintf(message, fmt, argptr);
    va_end(argptr);

    console_print(message);
}

void console_command(char* text_command)
{
    // TODO (Check if this bug still exists after switching to noclip) - FUCKING MEMORY BUG TEXT_COMMAND GETS NULL TERMINATED EARLY SOMETIMES
    char text_command_buffer[CONSOLE_COLS_MAX];
    strcpy_s(text_command_buffer, CONSOLE_COLS_MAX, text_command);//because text_command might point to read-only data

    if(*text_command_buffer == '\0')
    {
        return;
    }

    std::string cmd = std::string(text_command_buffer);
    std::string cmd_print_format = ">" + cmd + "\n";
    console_print(cmd_print_format.c_str());

    std::istringstream cmd_input_str(cmd);
    std::ostringstream cmd_output_str;
    get_console().execute(cmd_input_str, cmd_output_str);

    console_print(cmd_output_str.str().c_str());
}

void console_toggle()
{
    if(console_state == CONSOLE_HIDING || console_state == CONSOLE_SHOWING)
    {
        return;
    }

    if(console_state == CONSOLE_HIDDEN)
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        console_state = CONSOLE_SHOWING;
    }
    else if(console_state == CONSOLE_SHOWN)
    {
        // TODO pause game while shown
        SDL_SetRelativeMouseMode(SDL_TRUE);
        console_state = CONSOLE_HIDING;
    }
}

void console_update_messages()
{
    if(console_b_messages_dirty)
    {
        int msg_iterator = console_messages_read_cursor - 1;
        for(int row = 0;
            row < CONSOLE_ROWS_MAX;
            ++row)
        {
            // get line
            int line_len = 0;
            if(console_messages[msg_iterator] == '\n')
            {
                ++line_len;
                --msg_iterator;
            }
            for(char c = console_messages[msg_iterator];
                c != '\n' && c != '\0'; 
                c = console_messages[msg_iterator])
            {
                ++line_len;
                --msg_iterator;
                if(msg_iterator < 0)
                {
                    msg_iterator = CONSOLE_MAX_PRINT_MSGS - 1;
                }
            }
            // rebind vao
            {
                vtxt_setflags(VTXT_CREATE_INDEX_BUFFER|VTXT_USE_CLIPSPACE_COORDS);
                vtxt_backbuffersize(UI_LAYER_RESOLUTION_WIDTH, UI_LAYER_RESOLUTION_HEIGHT);

                vtxt_clear_buffer();
                vtxt_move_cursor(CONSOLE_INPUT_DRAW_X, CONSOLE_INPUT_DRAW_Y);
                for(int i = 0; i < line_len; ++i)
                {
                    int j = msg_iterator + i + 1;
                    if(j >= CONSOLE_MAX_PRINT_MSGS)
                    {
                        j -= CONSOLE_MAX_PRINT_MSGS;
                    }
                    char c = console_messages[j];
                    if(c != '\n')
                    {
                        vtxt_append_glyph(c, console_font_handle, CONSOLE_TEXT_SIZE);
                    }
                    else
                    {
                        vtxt_new_line(CONSOLE_INPUT_DRAW_X, console_font_handle);
                    }
                }
                vtxt_vertex_buffer vb = vtxt_grab_buffer();
                RebindMeshBufferObjects(console_messages_vaos[row], vb.vertex_buffer, vb.index_buffer, 
                    vb.vertices_array_count, vb.indices_array_count);
            }
        }

        console_b_messages_dirty = false;
    }
}

void console_update(float delta_time)
{
    if(!console_b_initialized || console_state == CONSOLE_HIDDEN)
    {
        return;
    }
    
    vtxt_setflags(VTXT_CREATE_INDEX_BUFFER|VTXT_USE_CLIPSPACE_COORDS);
    vtxt_backbuffersize(UI_LAYER_RESOLUTION_WIDTH, UI_LAYER_RESOLUTION_HEIGHT);

    switch(console_state)
    {
        case CONSOLE_SHOWN:
        {
            if(console_b_input_buffer_dirty)
            {
                // update input vao
                vtxt_clear_buffer();
                vtxt_move_cursor(CONSOLE_INPUT_DRAW_X, CONSOLE_INPUT_DRAW_Y);
                std::string input_text = ">" + std::string(console_input_buffer);
                vtxt_append_line(input_text.c_str(), console_font_handle, CONSOLE_TEXT_SIZE);
                vtxt_vertex_buffer vb = vtxt_grab_buffer();
                RebindMeshBufferObjects(console_inputtext_vao, vb.vertex_buffer, vb.index_buffer,
                    vb.vertices_array_count, vb.indices_array_count);
                console_b_input_buffer_dirty = false;
            }

            console_update_messages();
        } break;
        case CONSOLE_HIDING:
        {
            console_y -= CONSOLE_SCROLL_SPEED * delta_time;
            if(console_y < -1.f)
            {
                console_y = -1.f;
                console_state = CONSOLE_HIDDEN;
            }
        } break;
        case CONSOLE_SHOWING:
        {
            console_y += CONSOLE_SCROLL_SPEED * delta_time;
            if(console_y > CONSOLE_HEIGHT)
            {
                console_y = CONSOLE_HEIGHT;
                console_state = CONSOLE_SHOWN;
            }

            console_update_messages();
        } break;
    }
}

void console_render()
{
    if(!console_b_initialized || console_state == CONSOLE_HIDDEN)
    {
        return;
    }

    float console_translation_y = console_y - (float) CONSOLE_HEIGHT;
    mat4 con_transform = mat4(1.f);
    con_transform = glm::translate(con_transform, vec3(0.f, -console_translation_y, 0.f));

    // render console
    GLuint used_effect_enum = (GLuint) EFFECT_ASSET_ID::CONSOLE_UI;
    GLuint program = (GLuint)renderSystem->effects[used_effect_enum];
    glUseProgram(program);
        GLuint b_use_colour_loc = glGetUniformLocation(program, "b_use_colour");
        glUniform1i(b_use_colour_loc, true);
        GLuint matrix_model_loc = glGetUniformLocation(program, "matrix_model");
        glUniformMatrix4fv(matrix_model_loc, 1, GL_FALSE, (float*) &con_transform);
        glBindVertexArray(console_background_vao_id);
            GLuint ui_element_colour_loc = glGetUniformLocation(program, "ui_element_colour");
            glUniform4f(ui_element_colour_loc, 0.1f, 0.1f, 0.1f, 0.9f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(console_line_vao_id);
            glUniform4f(ui_element_colour_loc, 0.9f, 0.9f, 0.9f, 1.f);
            glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
    gl_has_errors();

    used_effect_enum = (GLuint) EFFECT_ASSET_ID::CONSOLE_TEXT_UI;
    program = (GLuint)renderSystem->effects[used_effect_enum];
    glUseProgram(program);
    gl_has_errors();
        // RENDER CONSOLE TEXT
        gl_has_errors();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, console_font_atlas.textureId);
        GLuint font_atlas_sampler_loc = glGetUniformLocation(program, "font_atlas_sampler");
        glUniform1i(font_atlas_sampler_loc, 1);
        gl_has_errors();

        // Input text visual
        con_transform[3][1] += (float)(5.f) / (buffer_dimensions.y*0.5f);
        GLuint text_colour_loc = glGetUniformLocation(program, "text_colour");
        glUniform3f(text_colour_loc, 1.f, 1.f, 1.f);
        matrix_model_loc = glGetUniformLocation(program, "matrix_model");
        glUniformMatrix4fv(matrix_model_loc, 1, GL_FALSE, (float*) &con_transform);
        if(console_inputtext_vao.indicesCount > 0)
        {
            glBindVertexArray(console_inputtext_vao.idVAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, console_inputtext_vao.idIBO);
                    glDrawElements(GL_TRIANGLES, console_inputtext_vao.indicesCount, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        gl_has_errors();
        // move transform matrix up a lil
        con_transform[3][1] += (float)(CONSOLE_TEXT_SIZE + 20) / (buffer_dimensions.y*0.5f);

        // Messages text visual
        glUniform3f(text_colour_loc, 0.9f, 0.9f, 0.9f);
        for(int i = 0; i < CONSOLE_ROWS_MAX; ++i)
        {
            MeshHandle m = console_messages_vaos[i];
            if(m.indicesCount > 0)
            {
                glUniformMatrix4fv(matrix_model_loc, 1, GL_FALSE, (float*) &con_transform);
                con_transform[3][1] += ((float) CONSOLE_TEXT_SIZE + 3.f) / (buffer_dimensions.y*0.5f);
                glBindVertexArray(m.idVAO);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.idIBO);
                        glDrawElements(GL_TRIANGLES, m.indicesCount, GL_UNSIGNED_INT, nullptr);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }
        }
    gl_has_errors();
    glUseProgram(0);
}

void console_scroll_up()
{
    int temp_cursor = console_messages_read_cursor - 1;
    char c = console_messages[temp_cursor];
    if(c == '\n')
    {
        --temp_cursor;
        if(temp_cursor < 0)
        {
            temp_cursor += CONSOLE_MAX_PRINT_MSGS;
        }
        c = console_messages[temp_cursor];
    }
    while(c != '\n' && c != '\0' && temp_cursor != console_messages_write_cursor)
    {
        --temp_cursor;
        if(temp_cursor < 0)
        {
            temp_cursor += CONSOLE_MAX_PRINT_MSGS;
        }
        c = console_messages[temp_cursor];
    }
    console_messages_read_cursor = temp_cursor + 1;
    if(console_messages_read_cursor < 0)
    {
        console_messages_read_cursor += CONSOLE_MAX_PRINT_MSGS;
    }
    console_b_messages_dirty = true;
}

void console_scroll_down()
{
    if(console_messages_read_cursor != console_messages_write_cursor)
    {
        int temp_cursor = console_messages_read_cursor;
        char c = console_messages[temp_cursor];
        while(c != '\n' && c != '\0' && temp_cursor != console_messages_write_cursor - 1)
        {
            ++temp_cursor;
            if(temp_cursor >= CONSOLE_MAX_PRINT_MSGS)
            {
                temp_cursor = 0;
            }
            c = console_messages[temp_cursor];
        }
        console_messages_read_cursor = temp_cursor + 1;
        if(console_messages_read_cursor > CONSOLE_MAX_PRINT_MSGS)
        {
            console_messages_read_cursor = 0;
        }
        console_b_messages_dirty = true;
    }
}

void console_keydown(SDL_KeyboardEvent& keyevent)
{
    SDL_Keycode keycode = keyevent.keysym.sym;

    // SPECIAL KEYS
    switch(keycode)
    {
        case SDLK_ESCAPE:
        {
            console_toggle();
            return;
        }
        // COMMAND
        case SDLK_RETURN:
        {
            // take current input buffer and use that as command
            console_command(console_input_buffer);
            memset(console_input_buffer, 0, console_input_buffer_count);
            console_input_cursor = 0;
            console_input_buffer_count = 0;
            console_b_input_buffer_dirty = true;
        }break;
        // Delete char before cursor
        case SDLK_BACKSPACE:
        {
            if(console_input_cursor > 0)
            {
                --console_input_cursor;
                console_input_buffer[console_input_cursor] = 0;
                --console_input_buffer_count;
                console_b_input_buffer_dirty = true;
            }
        }break;
        case SDLK_PAGEUP:
        {
            for(int i=0;i<10;++i)
            {
                console_scroll_up();
            }
        }break;
        case SDLK_PAGEDOWN:
        {
            for(int i=0;i<10;++i)
            {
                console_scroll_down();
            }
        }break;
        // TODO Move cursor left right
        case SDLK_LEFT:
        {

        }break;
        case SDLK_RIGHT:
        {

        }break;
        // TODO Flip through previously entered commands and fill command buffer w previous command
        case SDLK_UP:
        {
            console_scroll_up();
        }break;
        case SDLK_DOWN:
        {
            console_scroll_down();
        }break;
    }

    // CHECK MODIFIERS
    if(keyevent.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
    {
        if(97 <= keycode && keycode <= 122)
        {
            keycode -= 32;
        }
        else if(keycode == 50)
        {
            keycode = 64;
        }
        else if(49 <= keycode && keycode <= 53)
        {
            keycode -= 16;
        }
        else if(91 <= keycode && keycode <= 93)
        {
            keycode += 32;
        }
        else
        {
            switch(keycode)
            {
                case 48: { keycode = 41; } break;
                case 54: { keycode = 94; } break;
                case 55: { keycode = 38; } break;
                case 56: { keycode = 42; } break;
                case 57: { keycode = 40; } break;
                case 45: { keycode = 95; } break;
                case 61: { keycode = 43; } break;
                case 39: { keycode = 34; } break;
                case 59: { keycode = 58; } break;
                case 44: { keycode = 60; } break;
                case 46: { keycode = 62; } break;
                case 47: { keycode = 63; } break;
            }   
        }
    }

    // CHECK INPUT
    const int ASCII_SPACE = 32;
    const int ASCII_TILDE = 126;
    if((ASCII_SPACE <= keycode && keycode <= ASCII_TILDE))
    {
        if(console_input_buffer_count < CONSOLE_COLS_MAX)
        {
            console_input_buffer[console_input_cursor] = keycode;
            ++console_input_cursor;
            ++console_input_buffer_count;
            console_b_input_buffer_dirty = true;
        }
    }
}

bool console_is_shown()
{
    return console_b_initialized && console_state == CONSOLE_SHOWN;
}

bool console_is_hidden()
{
    return console_state == CONSOLE_HIDDEN;
}
