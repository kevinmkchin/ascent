#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

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

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("bug.png"),
			textures_path("eagle.png"),
            textures_path("spelunky.png") };

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
        shader_path("finalpass"),
		shader_path("sprite"),
    };

public:
	// Initialize the window
	bool init(SDL_Window* window);

    // Destroy resources associated to one or all entities created by the system
    void cleanUp();

	// Draw all entities
	void draw();

    // Call this when the display resolution (i.e. the window size) changes
    void updateBackBufferSize();
private:
    void initializeGlTextures();

    void initializeGlEffects();

    // Initialize the screen texture used as intermediate render target
    // The draw loop first renders to this texture
    bool initScreenTexture();

    void updateScreenTextureSize(i32 newWidth, i32 newHeight);

	// Internal drawing functions for each entity type
	void drawSprite(Entity entity, const mat3& projection);

	void finalDrawToScreen();

    mat3 createProjectionMatrix();

	// Window handle
    SDL_Window* window;
    i32 backbufferWidth = 0;
    i32 backbufferHeight = 0;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;
};

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
