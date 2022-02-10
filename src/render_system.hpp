#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

struct SpriteTransformSorting
{
    SpriteComponent sprite;
    TransformComponent transform;
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
private:
    void initializeGlTextures();

    void initializeGlEffects();

    // Initialize the screen texture used as intermediate render target
    // The draw loop first renders to this texture
    bool initScreenTexture();

    void updateScreenTextureSize(i32 newWidth, i32 newHeight);

	// Internal drawing functions for each entity type
    void drawSprite(TransformComponent entityTransform, SpriteComponent sprite, const mat3 &projection);

    void idontfuckingknow(std::vector<SpriteTransformSorting>& sortedSprites, const mat3 &projection);

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
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;
};

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
