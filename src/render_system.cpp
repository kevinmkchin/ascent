// internal
#include "render_system.hpp"

#include "tiny_ecs_registry.hpp"

void RenderSystem::drawSprite(Entity entity, const mat3 &projection)
{
    Motion& motion = registry.motions.get(entity);
    const SpriteComponent& sprite = registry.sprites.get(entity);

	Transform transform;
    vec2 scaledPosition = motion.position * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
    vec2 roundedPosition = vec2(floor(scaledPosition.x), floor(scaledPosition.y));
	transform.translate(roundedPosition);
    transform.rotate(motion.rotation * DEG2RAD);
    transform.scale(vec2(sprite.dimensions) * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL);
	transform.scale(motion.scale);

	const GLuint used_effect_enum = (GLuint) EFFECT_ASSET_ID::SPRITE;
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
    GLuint fcolor_loc = glGetUniformLocation(currProgram, "fcolor");
    glUniform3f(fcolor_loc, 1.f, 1.f, 1.f);
	gl_has_errors();

    LOCAL_PERSIST u32 spriteQuadVAO;
    LOCAL_PERSIST u32 spriteQuadVBO;
    LOCAL_PERSIST u32 spriteQuadIBO;
    if(!spriteQuadVAO)
    {
        u32 refQuadIndices[6] = {
                0, 1, 3,
                0, 3, 2
        };
        float refQuadVertices[16] = {
                //  x   y    u    v
                0.f, 0.f, 0.f, 0.f,
                1.f, 0.f, 1.f, 0.f,
                -0.f, 1.f, 0.f, 1.f,
                1.f, 1.f, 1.f, 1.f
        };

        glGenVertexArrays(1, &spriteQuadVAO);
        glBindVertexArray(spriteQuadVAO);
        glGenBuffers(1, &spriteQuadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, spriteQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, 4 /*bytes cuz float*/ * 16, refQuadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &spriteQuadIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteQuadIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * 6, refQuadIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); // Unbind the VAO;
    }

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    GLuint texture_id = texture_gl_handles[(GLuint)sprite.texId];
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Draw
    glBindVertexArray(spriteQuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteQuadIBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
	glDepthRange(0.00001f, 10.f);
	glClearColor(0.674f, 0.847f, 1.0f, 1.0f);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();

    for (Entity entWithSprite : registry.sprites.entities)
    {
        if (!registry.motions.has(entWithSprite)) continue;
        drawSprite(entWithSprite, projection_2D);
    }

	// Truely render to the screen
    finalDrawToScreen();
}

// draw the intermediate texture to the screen
void RenderSystem::finalDrawToScreen()
{
    // Setting shaders
    // get the wind texture, sprite mesh, and program
    glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::FINAL_PASS]);
    // Clearing backbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, backbufferWidth, backbufferHeight);
    glDepthRange(0, 10);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Enabling alpha channel for textures
    glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    LOCAL_PERSIST u32 finalQuadVAO;
    LOCAL_PERSIST u32 finalQuadVBO;
    LOCAL_PERSIST u32 finalQuadIBO;

    u32 refQuadIndices[6] = {
            0, 1, 3,
            0, 3, 2
    };
    float refQuadVertices[16] = {
            //  x   y    u    v
            -1.f, -1.f, 0.f, 0.f,
            1.f, -1.f, 1.f, 0.f,
            -1.f, 1.f, 0.f, 1.f,
            1.f, 1.f, 1.f, 1.f
    };

    if(!finalQuadVAO)
    {
        glGenVertexArrays(1, &finalQuadVAO);
        glBindVertexArray(finalQuadVAO);
        glGenBuffers(1, &finalQuadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, finalQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, 4 /*bytes cuz float*/ * 16, refQuadVertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &finalQuadIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * 6, refQuadIndices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); // Unbind the VAO;
    }

    LOCAL_PERSIST vec2 previousScreenSize = vec2();
    if(previousScreenSize != vec2(backbufferWidth, backbufferHeight))
    {
        previousScreenSize = vec2(backbufferWidth, backbufferHeight);
        float internal_ratio = (float)GAME_RESOLUTION_WIDTH / (float)GAME_RESOLUTION_HEIGHT;
        float screen_ratio = (float)backbufferWidth / (float)backbufferHeight;
        float finalOutputQuadVertices[16] = {
                -1.f, -1.f, 0.f, 0.f,
                1.f, -1.f, 1.f, 0.f,
                -1.f, 1.f, 0.f, 1.f,
                1.f, 1.f, 1.f, 1.f
        };
        if(screen_ratio > internal_ratio)
        {
            float w = backbufferHeight * internal_ratio;
            float f = (backbufferWidth - w) / backbufferWidth;
            finalOutputQuadVertices[0] = -1.f + f;
            finalOutputQuadVertices[4] = 1.f - f;
            finalOutputQuadVertices[8] = -1.f + f;
            finalOutputQuadVertices[12] = 1.f - f;
        }
        else
        {
            float h = backbufferWidth / internal_ratio;
            float f = (backbufferHeight - h) / backbufferHeight;
            finalOutputQuadVertices[1] = -1.f + f;
            finalOutputQuadVertices[5] = -1.f + f;
            finalOutputQuadVertices[9] = 1.f - f;
            finalOutputQuadVertices[13] = 1.f - f;
        }
        glBindVertexArray(finalQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, finalQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, 4 * 16, finalOutputQuadVertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * 6, refQuadIndices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        printf("Screen size changed\n");
    }

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);

    // Draw
    glBindVertexArray(finalQuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) FRAMEBUFFER_WIDTH;
	float bottom = (float) FRAMEBUFFER_HEIGHT;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}