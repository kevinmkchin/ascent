#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

class WorldSystem;
struct vtxt_font;

struct SpriteTransformPair
{
    u32 renderState;
    SpriteComponent* spritePtr;
    TransformComponent transform;
};

struct TextureHandle
{
    GLuint  textureId   = 0;        // ID for the texture in GPU memory
    i32     width       = 0;        // Width of the texture
    i32     height      = 0;        // Height of the texture
    GLenum  format      = GL_NONE;  // format / bitdepth of texture (GL_RGB would be 3 byte bit depth)
};

struct MeshHandle
{
    u32  idVAO          = 0;
    u32  idVBO          = 0;
    u32  idIBO          = 0;
    u32  indicesCount   = 0;
};

struct WorldText
{
    vec2 pos;
    u32 size;
    std::string text;
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

public:
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;
    std::array<GLuint, effect_count> effects;

    float elapsedTime = 0;
	// Initialize the window
	bool Init(SDL_Window* window, WorldSystem* world_sys_arg);

    // Destroy resources associated to one or all entities created by the system
    void CleanUp();

	// Draw all entities
	void Draw(float elapsed_ms);

    // Call this when the display resolution (i.e. the window size) changes
    void UpdateBackBufferSize();

    vec2 GetBackBufferSize()
    {
        return vec2((float)backbufferWidth, (float)backbufferHeight);
    }

    mat3 CreateGameProjectionMatrix();

    // Camera bounds
    vec2 cameraBoundMin;
    vec2 cameraBoundMax;

    // Background texture
    std::vector<TEXTURE_ASSET_ID> bgTexId = { TEXTURE_ASSET_ID::TEXTURE_COUNT };

    // UI
    TextureHandle   textLayer1FontAtlas;
    vec4            textLayer1Colour = vec4(1.f,1.f,1.f,1.f);
    MeshHandle      textLayer1VAO;
    TextureHandle   textLayer2FontAtlas;
    vec4            textLayer2Colour = vec4(1.f,1.f,1.f,1.f);
    MeshHandle      textLayer2VAO;
    TextureHandle   textLayer3FontAtlas;
    vec4            textLayer3Colour = vec4(1.f,1.f,1.f,1.f);
    MeshHandle      textLayer3VAO;
    TextureHandle   textLayer4FontAtlas;
    vec4            textLayer4Colour = vec4(1.f, 1.f, 1.f, 1.f);
    MeshHandle      textLayer4VAO;

    TextureHandle   worldTextFontAtlas;
    MeshHandle      worldTextVAO;
    std::vector<WorldText> worldTextsThisFrame;
    vtxt_font*      worldTextFontPtr;

    MeshHandle      healthBarBorder;

    MeshHandle      healthBar;
    float           healthPointsNormalized = 0.6f;//1.f;

    MeshHandle      expProgressBar;
    float           expProgressNormalized = 0.f; // range 0 to 1

    MeshHandle      mutationSelectBorder;
    MeshHandle      mutationSelectBox;
    bool            showMutationSelect = false;
    bool            showShopSelect = false;
    i8              mutationSelectionIndex = 1;

    float           currentTimeInSeconds = 0.f;

private:
    void InitializeGlTextures();

    void InitializeGlEffects();

    void InitializeUIStuff();

    // Initialize the screen texture used as intermediate render target
    // The draw loop first renders to this texture
    bool InitScreenTexture();

    void UpdateScreenTextureSize(i32 newWidth, i32 newHeight);
    
    // BATCH DRAWING
    void BatchDrawAllSprites(std::vector<SpriteTransformPair>& sortedSprites, const mat3& projection);

    void DrawMainMenuBackground(float elapsed_ms);

    void DrawAllBackgrounds(float elapsed_ms);

    void DrawBackground(TEXTURE_ASSET_ID texId, float offset);

    void DrawUI();

	void FinalDrawToScreen();

    // World handle
    WorldSystem* world;

	// Window handle
    SDL_Window* window;
    i32 backbufferWidth = 0;
    i32 backbufferHeight = 0;
    const i32 FRAMEBUFFER_WIDTH = GAME_RESOLUTION_WIDTH * FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
    const i32 FRAMEBUFFER_HEIGHT = GAME_RESOLUTION_HEIGHT * FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;

    Transform cameraTransform;

	// Screen texture handles
	GLuint gameFrameBuffer;
	GLuint offScreenRenderBufferColor;
	GLuint offScreenRenderBufferDepth;

    GLuint uiFrameBuffer;
    GLuint offScreenUiBufferColor;
    GLuint offScreenUiBufferDepth;

};

bool LoadEffectFromFile(const std::string& vs_path, const std::string& fs_path, GLuint& out_program);

void CreateTextureFromBitmap(TextureHandle&    texture,
                             unsigned char*    bitmap,
                             u32               bitmap_width,
                             u32               bitmap_height,
                             GLenum            target_format,
                             GLenum            source_format,
                             GLenum            filter_mode = GL_LINEAR);

void CreateMeshVertexArray(MeshHandle& mesh,
                           float* vertices,
                           u32* indices,
                           u32 vertices_array_count,
                           u32 indices_array_count,
                           u8 vertex_attrib_size = 3,
                           u8 texture_attrib_size = 2,
                           u8 normal_attrib_size = 3,
                           GLenum draw_usage = GL_DYNAMIC_DRAW);

void RebindMeshBufferObjects(MeshHandle& mesh,
                             float* vertices,
                             u32* indices,
                             u32 vertices_array_count,
                             u32 indices_array_count,
                             GLenum draw_usage = GL_DYNAMIC_DRAW);
