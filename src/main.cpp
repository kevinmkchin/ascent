
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

#include <SDL.h>

// stlib
#include <chrono>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "input.hpp"
#include "player_system.hpp"
#include "sprite_system.hpp"
#include "ui_system.hpp"
//#include "timer.h"

#define TINY_ECS_LIB_IMPLEMENTATION
#include "tiny_ecs.hpp"
#include "item_holder_system.hpp"

#define WINDOW_TITLE "ASCENT"
#define WINDOW_INITIAL_WIDTH 800
#define WINDOW_INITIAL_HEIGHT 600

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
    SDL_SetWindowResizable(window, SDL_TRUE);
    printf("SDL window created.\n");

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
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
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
    float GlobalPauseForSeconds = 0.f;
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
    PlayerSystem playerSystem;
    playerSystem.GlobalPauseForSeconds = &GlobalPauseForSeconds;
    SpriteSystem spriteSystem;
    ItemHolderSystem itemHolderSystem;
	AISystem ai;
    UISystem ui;

    // Initialize SDL window and OpenGL context
    if(!SDLInitialize()) return EXIT_FAILURE;
    if(!OpenGLInitialize()) return EXIT_FAILURE;

	// Initialize the main systems
    renderer.Init(window, &world);
	  world.init(&renderer, &playerSystem);
    world.GlobalPauseForSeconds = &GlobalPauseForSeconds;
    ui.Init(&renderer, &world, &playerSystem);
    ui.GlobalPauseForSeconds = &GlobalPauseForSeconds;

	// Variable timestep loop
	auto t = Clock::now();
	while (!world.is_over()) {
        //printf("swap window: %f seconds\n", timer::timestamp());
        //printf("\n");
		// Process system messages
        world.SDLProcessEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;
        float deltaTime = elapsed_ms / 1000.f; // elapsed time in SECONDS
        if(deltaTime > 0.1f) { continue; } // if delta time is too large, will cause glitches

        if(GlobalPauseForSeconds > 0.f || world.gamePaused)
        {
            if(!world.gamePaused) { GlobalPauseForSeconds -= deltaTime; }
            playerSystem.PausedStep(deltaTime);
        }
        else
        {
            world.UpdateMode();
            world.step(deltaTime);
            //printf("world.Step: %f seconds\n", timer::timestamp());
            if(world.GetCurrentMode() == MODE_INGAME)
            {   
                playerSystem.PrePhysicsStep(deltaTime);
                physics.step(deltaTime);
                //printf("physics.Step: %f seconds\n", timer::timestamp());
                ai.Step(deltaTime);
                //printf("ai.Step: %f seconds\n", timer::timestamp());
                playerSystem.Step(deltaTime);
                //printf("playerSystem.Step: %f seconds\n", timer::timestamp());
                itemHolderSystem.Step(deltaTime);
                //printf("itemHolderSystem.Step: %f seconds\n", timer::timestamp());
                spriteSystem.Step(deltaTime);
                //printf("spriteSystem.Step: %f seconds\n", timer::timestamp());
                world.handle_collisions();
                //printf("handle_collisions: %f seconds\n", timer::timestamp());
            }
        }

        ui.Step(deltaTime);

        Input::ResetControllerStates();
        Input::ResetKeyboardStates();

        renderer.Draw();
        //printf("Draw: %f seconds\n", timer::timestamp());
        //float fps = 1 / deltaTime;
        //printf("%f DELTA TIME \n", fps);

        SDL_GL_SwapWindow(window);
	}

    // Clean up
    world.cleanUp();
    renderer.CleanUp();

    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(openglContext);
    SDL_Quit();

	return EXIT_SUCCESS;
}
