#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <SDL.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tiny_ecs.hpp"

// Note(Kevin): This is the resolution of the game. I'm not expecting this
// to change during runtime, so I've made these as #define instead of variables.
// (e.g. Celeste has an internal resolution of 320x180 regardless of the window size)
// FRAMEBUFFER_PIXELS_PER_GAME_PIXEL is the ratio between internal frame buffer and
// the game resolution.
#define GAME_RESOLUTION_WIDTH 240
#define GAME_RESOLUTION_HEIGHT 180
#define FRAMEBUFFER_PIXELS_PER_GAME_PIXEL 1
#define UI_LAYER_RESOLUTION_WIDTH (GAME_RESOLUTION_WIDTH * 6)
#define UI_LAYER_RESOLUTION_HEIGHT (GAME_RESOLUTION_HEIGHT * 6)

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return data_path() + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};
inline std::string level_path(const std::string& name) {return data_path() + "/level/" + std::string(name);};
inline std::string font_path(const std::string& name) {return data_path() + "/fonts/" + std::string(name);};

#define ASCENT_PI 3.14159265358979323846f
#define HALF_PI 1.570796f
#define QUARTER_PI 0.785398f

#define DEG2RAD 0.0174532925f  // degrees * DEG2RAD = radians
#define RAD2DEG 57.2958f       // radians * RAD2DEG = degrees
#define ASCENT_abs(a) ((a) < 0.f ? (-(a)) : (a))

#define ARRAY_COUNT(a) (sizeof(a) / (sizeof(a[0])))

#define INTERNAL static
#define LOCAL_PERSIST static

typedef uint8_t       u8;
typedef uint16_t      u16;
typedef uint32_t      u32;
typedef uint64_t      u64;
typedef int8_t        i8;
typedef int16_t       i16;
typedef int32_t       i32;
typedef int64_t       i64;
typedef uint_fast8_t  u8f;
typedef uint_fast16_t u16f;
typedef uint_fast32_t u32f;
typedef int_fast8_t   i8f;
typedef int_fast16_t  i16f;
typedef int_fast32_t  i32f;
typedef i16           bool16;
typedef i32           bool32;

struct shortvec2
{
    i16 x;
    i16 y;

    shortvec2()
        : x(0)
        , y(0)
    {}

    shortvec2(i16 inx, i16 iny)
        : x(inx)
        , y(iny)
    {}

    shortvec2(glm::vec2 glmvec)
    	: x((i16) glmvec.x)
    	, y((i16) glmvec.y)
    {}

    operator glm::vec2() const { return glm::vec2((float)x, (float)y); }
};

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
