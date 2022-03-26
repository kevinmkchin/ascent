#pragma once

#include <SDL_events.h>
#include <noclip.h>

noclip::console& get_console();

struct TextureHandle;
struct shader_t;
struct vtxt_font;
class RenderSystem;

void console_initialize(vtxt_font* in_console_font_handle, TextureHandle in_console_font_atlas, RenderSystem* renderer);
void console_toggle();
void console_update(float delta_time);
void console_render();

void console_print(const char* message);
void console_printf(const char* fmt, ...);
void console_command(char* text_command);

void console_keydown(SDL_KeyboardEvent& keyevent);
void console_scroll_up();
void console_scroll_down();
bool console_is_shown();
bool console_is_hidden();