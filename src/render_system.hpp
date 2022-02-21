#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

struct SpriteTransformPair
{
    u32 renderState;
    SpriteComponent sprite;
    TransformComponent transform;
};

struct texture_t
{
    GLuint  texture_id  = 0;        // ID for the texture in GPU memory
    i32     width       = 0;        // Width of the texture
    i32     height      = 0;        // Height of the texture
    GLenum  format      = GL_NONE;  // format / bitdepth of texture (GL_RGB would be 3 byte bit depth)
};

struct mesh_t
{
    u32  id_vao          = 0;
    u32  id_vbo          = 0;
    u32  id_ibo          = 0;
    u32  indices_count   = 0;
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;
    std::array<GLuint, effect_count> effects;

public:
	// Initialize the window
	bool init(SDL_Window* window);

    // Destroy resources associated to one or all entities created by the system
    void cleanUp();

	// Draw all entities
	void draw();

    // Call this when the display resolution (i.e. the window size) changes
    void updateBackBufferSize();

    // Camera bounds
    vec2 cameraBoundMin;
    vec2 cameraBoundMax;

    // Background texture
    TEXTURE_ASSET_ID bgTexId = TEXTURE_ASSET_ID::TEXTURE_COUNT;
private:
    void initializeGlTextures();

    void initializeGlEffects();

    // Initialize the screen texture used as intermediate render target
    // The draw loop first renders to this texture
    bool initScreenTexture();

    void updateScreenTextureSize(i32 newWidth, i32 newHeight);

	// Internal drawing functions for each entity type
    void drawSprite(TransformComponent entityTransform, SpriteComponent sprite, const mat3 &projection);

    // BATCH DRAWING
    void BatchDrawAllSprites(std::vector<SpriteTransformPair>& sortedSprites, const mat3 &projection);

    void drawBackground();

	void finalDrawToScreen();

    mat3 createProjectionMatrix();

	// Window handle
    SDL_Window* window;
    i32 backbufferWidth = 0;
    i32 backbufferHeight = 0;
    const i32 FRAMEBUFFER_WIDTH = GAME_RESOLUTION_WIDTH * FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
    const i32 FRAMEBUFFER_HEIGHT = GAME_RESOLUTION_HEIGHT * FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;

	// Screen texture handles
	GLuint game_frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

    GLuint ui_frame_buffer;
    GLuint off_screen_ui_buffer_color;
    GLuint off_screen_ui_buffer_depth;

public:
    // UI
    texture_t   font_atlas;
    mesh_t      console_inputtext_vao;

private:

};

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path, GLuint& out_program);

void gl_create_from_bitmap(texture_t&        texture,
                           unsigned char*    bitmap,
                           u32               bitmap_width,
                           u32               bitmap_height,
                           GLenum            target_format,
                           GLenum            source_format);

void gl_create_mesh(mesh_t& mesh,
                    float* vertices,
                    u32* indices,
                    u32 vertices_array_count,
                    u32 indices_array_count,
                    u8 vertex_attrib_size = 3,
                    u8 texture_attrib_size = 2,
                    u8 normal_attrib_size = 3,
                    GLenum draw_usage = GL_DYNAMIC_DRAW);

void gl_rebind_buffer_objects(mesh_t& mesh,
                              float* vertices,
                              u32* indices,
                              u32 vertices_array_count,
                              u32 indices_array_count,
                              GLenum draw_usage = GL_DYNAMIC_DRAW);
