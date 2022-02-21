// internal
#include "render_system.hpp"

#include <array>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// This creates circular header inclusion, that is quite bad.
#include "tiny_ecs_registry.hpp"

// stlib
#include <iostream>
#include <sstream>

// World initialization
bool RenderSystem::init(SDL_Window* window_arg)
{
	this->window = window_arg;

    SDL_GL_GetDrawableSize(window, &backbufferWidth, &backbufferHeight);

	// Create a frame buffer
	game_frame_buffer = 0;
	glGenFramebuffers(1, &game_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, game_frame_buffer);
	gl_has_errors();

	ui_frame_buffer = 0;
	glGenFramebuffers(1, &ui_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ui_frame_buffer);
	gl_has_errors();

	// Hint: Ask your TA for how to setup pretty OpenGL error callbacks.
	// This can not be done in mac os, so do not enable
	// it unless you are on Linux or Windows. You will need to change the window creation
	// code to use OpenGL 4.3 (not suported on mac) and add additional .h and .cpp
	// glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

	// We are not really using VAO's but without at least one bound we will crash in
	// some systems.
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	gl_has_errors();

	initScreenTexture();
    initializeGlTextures();
	initializeGlEffects();
	InitializeUIStuff();

	return true;
}

void RenderSystem::InitializeUIStuff()
{
    CreateMeshVertexArray(textLayer1VAO, nullptr, nullptr, 0, 0, 2, 2, 0, GL_DYNAMIC_DRAW);
}

void RenderSystem::initializeGlTextures()
{
    glGenTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());

    for(uint i = 0; i < texture_paths.size(); i++)
    {
		const std::string& path = texture_paths[i];
		ivec2& dimensions = texture_dimensions[i];

		stbi_uc* data;
		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

		if (data == NULL)
		{
			const std::string message = "Could not load the file " + path + ".";
			fprintf(stderr, "%s", message.c_str());
			assert(false);
		}
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl_has_errors();
		stbi_image_free(data);
    }
	gl_has_errors();
}

void RenderSystem::initializeGlEffects()
{
	for(uint i = 0; i < effect_paths.size(); i++)
	{
		const std::string vertex_shader_name = effect_paths[i] + ".vert";
		const std::string fragment_shader_name = effect_paths[i] + ".frag";

		bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, effects[i]);
		assert(is_valid && (GLuint)effects[i] != 0);
	}
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, game_frame_buffer);

	glGenTextures(1, &off_screen_render_buffer_color);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl_has_errors();

	glGenRenderbuffers(1, &off_screen_render_buffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, off_screen_render_buffer_color, 0);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
	gl_has_errors();

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);


    glBindFramebuffer(GL_FRAMEBUFFER, ui_frame_buffer);

	glGenTextures(1, &off_screen_ui_buffer_color);
	glBindTexture(GL_TEXTURE_2D, off_screen_ui_buffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, UI_LAYER_RESOLUTION_WIDTH, UI_LAYER_RESOLUTION_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_has_errors();

	glGenRenderbuffers(1, &off_screen_ui_buffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, off_screen_ui_buffer_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, off_screen_ui_buffer_color, 0);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, UI_LAYER_RESOLUTION_WIDTH, UI_LAYER_RESOLUTION_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_ui_buffer_depth);
	gl_has_errors();

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);


	return true;
}

void RenderSystem::updateScreenTextureSize(i32 newWidth, i32 newHeight)
{
    glBindFramebuffer(GL_FRAMEBUFFER, game_frame_buffer);
    glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, newWidth, newHeight);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Failed to change size of Internal FrameBuffer Object.\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSystem::cleanUp()
{
    // Don't need to free gl resources since they last for as long as the program,
    // but it's polite to clean after yourself.
	glDeleteTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());
	glDeleteTextures(1, &off_screen_render_buffer_color);
	glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
	glDeleteTextures(1, &off_screen_ui_buffer_color);
	glDeleteRenderbuffers(1, &off_screen_ui_buffer_depth);
	gl_has_errors();

    for(uint i = 0; i < effect_count; i++) {
        glDeleteProgram(effects[i]);
    }
    // delete allocated resources
    glDeleteFramebuffers(1, &game_frame_buffer);
    glDeleteFramebuffers(1, &ui_frame_buffer);
    gl_has_errors();
}

void RenderSystem::updateBackBufferSize()
{
    SDL_GL_GetDrawableSize(window, &backbufferWidth, &backbufferHeight);
}

bool gl_compile_shader(GLuint shader)
{
	glCompileShader(shader);
	gl_has_errors();
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		std::vector<char> log(log_len);
		glGetShaderInfoLog(shader, log_len, &log_len, log.data());
		glDeleteShader(shader);

		gl_has_errors();

		fprintf(stderr, "GLSL: %s", log.data());
		return false;
	}

	return true;
}

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path, GLuint& out_program)
{
	// Opening files
	std::ifstream vs_is(vs_path);
	std::ifstream fs_is(fs_path);
	if (!vs_is.good() || !fs_is.good())
	{
		fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
		assert(false);
		return false;
	}

	// Reading sources
	std::stringstream vs_ss, fs_ss;
	vs_ss << vs_is.rdbuf();
	fs_ss << fs_is.rdbuf();
	std::string vs_str = vs_ss.str();
	std::string fs_str = fs_ss.str();
	const char* vs_src = vs_str.c_str();
	const char* fs_src = fs_str.c_str();
	GLsizei vs_len = (GLsizei)vs_str.size();
	GLsizei fs_len = (GLsizei)fs_str.size();

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_src, &vs_len);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_src, &fs_len);
	gl_has_errors();

	// Compiling
	if (!gl_compile_shader(vertex))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}
	if (!gl_compile_shader(fragment))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}

	// Linking
	out_program = glCreateProgram();
	glAttachShader(out_program, vertex);
	glAttachShader(out_program, fragment);
	glLinkProgram(out_program);
	gl_has_errors();

	{
		GLint is_linked = GL_FALSE;
		glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
		if (is_linked == GL_FALSE)
		{
			GLint log_len;
			glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
			gl_has_errors();

			fprintf(stderr, "Link error: %s", log.data());
			assert(false);
			return false;
		}
	}

	// No need to carry this around. Keeping these objects is only useful if we recycle
	// the same shaders over and over, which we don't, so no need and this is simpler.
	glDetachShader(out_program, vertex);
	glDetachShader(out_program, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	gl_has_errors();

	return true;
}

void CreateTextureFromBitmap(TextureHandle&    texture,
                             unsigned char*    bitmap,
                             u32               bitmap_width,
                             u32               bitmap_height,
                             GLenum            target_format,
                             GLenum            source_format)
{
    texture.width = bitmap_width;
    texture.height = bitmap_height;
    texture.format = source_format;

    glGenTextures(1, &texture.texture_id);                              // generate texture and grab texture id
    glBindTexture(GL_TEXTURE_2D, texture.texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    	// wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // filtering (e.g. GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
            GL_TEXTURE_2D,            // texture target type
            0,                        // level-of-detail number n = n-th mipmap reduction image
            target_format,            // format of data to store (target): num of color components
            bitmap_width,             // texture width
            bitmap_height,            // texture height
            0,                        // must be 0 (legacy)
            source_format,            // format of data being loaded (source)
            GL_UNSIGNED_BYTE,         // data type of the texture data
            bitmap);                  // data
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CreateMeshVertexArray(MeshHandle& mesh,
                    	   float* vertices,
                    	   u32* indices,
                    	   u32 vertices_array_count,
                    	   u32 indices_array_count,
                    	   u8 vertex_attrib_size,
                    	   u8 texture_attrib_size,
                    	   u8 normal_attrib_size,
                    	   GLenum draw_usage)
{
    u8 stride = 0;
    if(texture_attrib_size)
    {
        stride += vertex_attrib_size + texture_attrib_size;
        if(normal_attrib_size)
        {
            stride += normal_attrib_size;
        }
    }

    mesh.indices_count = indices_array_count;

    glGenVertexArrays(1, &mesh.id_vao);
    glBindVertexArray(mesh.id_vao);
    glGenBuffers(1, &mesh.id_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.id_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * vertices_array_count, vertices, draw_usage);
    glVertexAttribPointer(0, vertex_attrib_size, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0); // vertex pointer
    glEnableVertexAttribArray(0);
    if(texture_attrib_size > 0)
    {
        glVertexAttribPointer(1, texture_attrib_size, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * vertex_attrib_size)); 
        glEnableVertexAttribArray(1);
        if(normal_attrib_size > 0)
        {
            glVertexAttribPointer(2, normal_attrib_size, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * (vertex_attrib_size + texture_attrib_size)));
            glEnableVertexAttribArray(2);
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &mesh.id_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.id_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indices_array_count, indices, draw_usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RebindMeshBufferObjects(MeshHandle& mesh,
						     float* vertices,
                             u32* indices,
                             u32 vertices_array_count,
                             u32 indices_array_count,
                             GLenum draw_usage)
{
    if(mesh.id_vbo == 0 || mesh.id_ibo == 0)
    {
        return;
    }

    mesh.indices_count = indices_array_count;
    glBindVertexArray(mesh.id_vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.id_vbo);
            glBufferData(GL_ARRAY_BUFFER, 4 * vertices_array_count, vertices, draw_usage);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.id_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indices_array_count, indices, draw_usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
