
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "input.hpp"

#include <SDL.h>

#define WINDOW_TITLE "ASCENT"
#define WINDOW_INITIAL_WIDTH 1280
#define WINDOW_INITIAL_HEIGHT 720

INTERNAL SDL_Window* window = nullptr;
INTERNAL SDL_GLContext openglContext = nullptr;

INTERNAL bool SDLInitialize()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL failed to initialize.\n");
        return false;
    }

    // OpenGL Context Attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Setup SDL Window
    if ((window = SDL_CreateWindow(
            WINDOW_TITLE,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WINDOW_INITIAL_WIDTH,
            WINDOW_INITIAL_HEIGHT,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    )) == nullptr)
    {
        printf("SDL window failed to create.\n");
        return false;
    }
    SDL_SetWindowResizable(window, SDL_TRUE); // TODO(Kevin): make window resizable

    if ((openglContext = SDL_GL_CreateContext(window)) == nullptr)
    {
        printf("Failed to create OpenGL context with SDL.\n");
        return false;
    }
    printf("OpenGL context created.\n");

    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    return true;
}

INTERNAL bool OpenGLInitialize()
{
    if (gl3w_init())
    {
        printf("Failed to initialize OpenGL through gl3w...\n");
        return false;
    }
    printf("gl3w initialized.\n");

    SDL_GL_SetSwapInterval(1); // vsync

    return true;
}

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main(int argc, char* argv[])
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem ai;

    // Initialize SDL window and OpenGL context
    if(!SDLInitialize()) return EXIT_FAILURE;
    if(!OpenGLInitialize()) return EXIT_FAILURE;

	// Initialize the main systems
	renderer.init(window);
	world.init(&renderer);

	// Variable timestep loop
	auto t = Clock::now();
	while (!world.is_over()) {
		// Process system messages
        world.SDLProcessEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		world.step(elapsed_ms);
		ai.step(elapsed_ms);
		physics.step(elapsed_ms);
		world.handle_collisions();

        Input::ResetControllerStates();
        Input::ResetKeyboardStates();

		renderer.draw();
        SDL_GL_SwapWindow(window);
	}

    // Clean up
    world.cleanUp();
    renderer.cleanUp();

    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(openglContext);
    SDL_Quit();

	return EXIT_SUCCESS;
}
