// internal
#include "render_system.hpp"

#include "tiny_ecs_registry.hpp"

// Using counting sort to sort the elements in the basis of significant places
void countingSort(std::vector<SpriteTransformSorting>& array, int size, int place) {
    const int max = 10;
    std::vector<SpriteTransformSorting> output(size);
    int count[max];

    for (int i = 0; i < max; ++i)
        count[i] = 0;

    // Calculate count of elements
    for (int i = 0; i < size; i++)
        count[((u32) array[i].sprite.texId / place) % 10]++;

    // Calculate cumulative count
    for (int i = 1; i < max; i++)
        count[i] += count[i - 1];

    // Place the elements in sorted order
    for (int i = size - 1; i >= 0; i--) {
        output[count[((u32) array[i].sprite.texId / place) % 10] - 1] = array[i];
        count[((u32) array[i].sprite.texId / place) % 10]--;
    }

    for (int i = 0; i < size; i++)
        array[i] = output[i];
}

// Main function to implement radix sort
void radixsort(std::vector<SpriteTransformSorting>& array)
{
    int size = array.size();

    // Get maximum element
    u32 max = (u32) array[0].sprite.texId;
    for (int i = 1; i < size; i++)
        if ((u32) array[i].sprite.texId > max)
            max = (u32) array[i].sprite.texId;

    // Apply counting sort to sort elements based on place value.
    for (int place = 1; max / place > 0; place *= 10)
        countingSort(array, size, place);
}

//int array[] = {121, 432, 564, 23, 1, 45, 788};
//int n = sizeof(array) / sizeof(array[0]);
//radixsort(array, n);

void RenderSystem::drawSprite(const TransformComponent entityTransform, const SpriteComponent sprite, const mat3 &projection)
{
	Transform transform;
    vec2 scaledPosition = entityTransform.position * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
	transform.translate(scaledPosition - (entityTransform.center * entityTransform.scale * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL));
    transform.rotate(entityTransform.rotation * DEG2RAD);
    transform.scale(vec2(sprite.dimensions) * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL);
	transform.scale(entityTransform.scale);

    Transform cameraTransform;
    Entity player = registry.players.entities[0];
    TransformComponent& playerTransform = registry.transforms.get(player);
    float playerPositionX = clamp(playerTransform.position.x, cameraBoundMin.x, cameraBoundMax.x);
    float playerPositionY = clamp(playerTransform.position.y, cameraBoundMin.y, cameraBoundMax.y);
    playerPositionX = playerPositionX - (GAME_RESOLUTION_WIDTH / 2.0f);
    playerPositionY = playerPositionY - (GAME_RESOLUTION_HEIGHT / 2.0f);
    vec2 playerPosition = vec2(playerPositionX, playerPositionY);
    vec2 scaledPlayerPosition = playerPosition * (float)FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
    cameraTransform.translate(-scaledPlayerPosition);

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
    GLuint camera_loc = glGetUniformLocation(currProgram, "cameraTransform");
    glUniformMatrix3fv(camera_loc, 1, GL_FALSE, (float*)&cameraTransform.mat);
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

void RenderSystem::drawBackground()
{
    const GLuint used_effect_enum = (GLuint) EFFECT_ASSET_ID::BACKGROUND;
    const GLuint program = (GLuint)effects[used_effect_enum];

    glUseProgram(program);
    gl_has_errors();

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    gl_has_errors();

    LOCAL_PERSIST u32 bgQuadVAO;
    LOCAL_PERSIST u32 bgQuadVBO;
    LOCAL_PERSIST u32 bgQuadIBO;
    if(!bgQuadVAO)
    {
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

        glGenVertexArrays(1, &bgQuadVAO);
        glBindVertexArray(bgQuadVAO);
        glGenBuffers(1, &bgQuadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, bgQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, 4 /*bytes cuz float*/ * 16, refQuadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &bgQuadIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgQuadIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * 6, refQuadIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); // Unbind the VAO;
    }

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    GLuint texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::BG1];
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Draw
    glBindVertexArray(bgQuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgQuadIBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    gl_has_errors();
}

void RenderSystem::idontfuckingknow(std::vector<SpriteTransformSorting>& sortedSprites, const mat3 &projection)
{
    LOCAL_PERSIST u32 bgQuadVAO;
    LOCAL_PERSIST u32 bgQuadVBO;
    LOCAL_PERSIST u32 bgQuadIBO;
    if(!bgQuadVAO)
    {
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

        glGenVertexArrays(1, &bgQuadVAO);
        glBindVertexArray(bgQuadVAO);
        glGenBuffers(1, &bgQuadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, bgQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, 4 /*bytes cuz float*/ * 16, refQuadVertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &bgQuadIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgQuadIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * 6, refQuadIndices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); // Unbind the VAO;
    }

    Transform cameraTransform;
    Entity player = registry.players.entities[0];
    TransformComponent& playerTransform = registry.transforms.get(player);
    float playerPositionX = clamp(playerTransform.position.x, cameraBoundMin.x, cameraBoundMax.x);
    float playerPositionY = clamp(playerTransform.position.y, cameraBoundMin.y, cameraBoundMax.y);
    playerPositionX = playerPositionX - (GAME_RESOLUTION_WIDTH / 2.0f);
    playerPositionY = playerPositionY - (GAME_RESOLUTION_HEIGHT / 2.0f);
    vec2 playerPosition = vec2(playerPositionX, playerPositionY);
    vec2 scaledPlayerPosition = playerPosition * (float)FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
    cameraTransform.translate(-scaledPlayerPosition);

    u32 renderState = (u32) sortedSprites[0].sprite.texId;
    std::vector<float> vertices(16 * sortedSprites.size());
    u32 verticesCount = 0;
    std::vector<u32> indices(6 * sortedSprites.size());
    u32 indicesCount = 0;

    for(auto& sprtr : sortedSprites)
    {
        if(renderState != (u32)sprtr.sprite.texId)
        {
            Transform transform;

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
            GLuint camera_loc = glGetUniformLocation(currProgram, "cameraTransform");
            glUniformMatrix3fv(camera_loc, 1, GL_FALSE, (float*)&cameraTransform.mat);
            GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
            glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
            GLuint fcolor_loc = glGetUniformLocation(currProgram, "fcolor");
            glUniform3f(fcolor_loc, 1.f, 1.f, 1.f);
            gl_has_errors();

            // flush
            glBindVertexArray(bgQuadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, bgQuadVBO);
            glBufferData(GL_ARRAY_BUFFER, 4 * verticesCount, vertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgQuadIBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indicesCount, indices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            // Bind our texture in Texture Unit 0
            glActiveTexture(GL_TEXTURE0);
            GLuint texture_id = texture_gl_handles[(GLuint)renderState]; // TODO don't use renderState
            glBindTexture(GL_TEXTURE_2D, texture_id);

            // Draw
            glBindVertexArray(bgQuadVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgQuadIBO);
            glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            verticesCount = 0;
            indicesCount = 0;

            renderState = (u32) sprtr.sprite.texId;
        }

        vertices[verticesCount + 0] = sprtr.transform.position.x - sprtr.transform.center.x;
        vertices[verticesCount + 1] = sprtr.transform.position.y - sprtr.transform.center.y;
        vertices[verticesCount + 2] = 0.f; // U
        vertices[verticesCount + 3] = 0.f; // V
        vertices[verticesCount + 4] = sprtr.transform.position.x - sprtr.transform.center.x + sprtr.sprite.dimensions.x;
        vertices[verticesCount + 5] = sprtr.transform.position.y - sprtr.transform.center.y;
        vertices[verticesCount + 6] = 1.f; // U
        vertices[verticesCount + 7] = 0.f; // V
        vertices[verticesCount + 8] = sprtr.transform.position.x - sprtr.transform.center.x;
        vertices[verticesCount + 9] = sprtr.transform.position.y - sprtr.transform.center.y + sprtr.sprite.dimensions.y;
        vertices[verticesCount + 10] = 0.f; // U
        vertices[verticesCount + 11] = 1.f; // V
        vertices[verticesCount + 12] = sprtr.transform.position.x - sprtr.transform.center.x + sprtr.sprite.dimensions.x;
        vertices[verticesCount + 13] = sprtr.transform.position.y - sprtr.transform.center.y + sprtr.sprite.dimensions.y;
        vertices[verticesCount + 14] = 1.f; // U
        vertices[verticesCount + 15] = 1.f; // V

        indices[indicesCount + 0] = 4*(indicesCount/6) + 0;
        indices[indicesCount + 1] = 4*(indicesCount/6) + 1;
        indices[indicesCount + 2] = 4*(indicesCount/6) + 3;
        indices[indicesCount + 3] = 4*(indicesCount/6) + 0;
        indices[indicesCount + 4] = 4*(indicesCount/6) + 3;
        indices[indicesCount + 5] = 4*(indicesCount/6) + 2;

        verticesCount += 16;
        indicesCount += 6;
    }

    Transform transform;

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
    GLuint camera_loc = glGetUniformLocation(currProgram, "cameraTransform");
    glUniformMatrix3fv(camera_loc, 1, GL_FALSE, (float*)&cameraTransform.mat);
    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
    GLuint fcolor_loc = glGetUniformLocation(currProgram, "fcolor");
    glUniform3f(fcolor_loc, 1.f, 1.f, 1.f);
    gl_has_errors();

    // flush
    glBindVertexArray(bgQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bgQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * verticesCount, vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgQuadIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indicesCount, indices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    GLuint texture_id = texture_gl_handles[(GLuint)renderState]; // TODO don't use renderState
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Draw
    glBindVertexArray(bgQuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgQuadIBO);
    glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    verticesCount = 0;
    indicesCount = 0;
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
	//glClearColor(0.674f, 0.847f, 1.0f, 1.0f);
	glClearColor(27.f/255.f, 28.f/255.f, 23.f/255.f, 1.0f);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();

    // DRAW BACKGROUND
    drawBackground();

    std::vector<SpriteTransformSorting> sorting;
    sorting.resize(registry.sprites.size());
    for(u32 i = 0; i < registry.sprites.size(); ++i)
    {
        auto e = registry.sprites.entities[i];
        SpriteTransformSorting s;
        s.sprite = registry.sprites.get(e);
        s.transform = registry.transforms.get(e);
        sorting[i] = s;
    }
    radixsort(sorting);
    idontfuckingknow(sorting, projection_2D);

//    // DRAW PLAYER
//    for(Entity e : registry.players.entities)
//    {
//        drawSprite(registry.transforms.get(e), registry.sprites.get(e), projection_2D);
//    }
//
//    // DRAW ENEMIES
//    for(Entity e : registry.enemy.entities)
//    {
//        drawSprite(registry.transforms.get(e), registry.sprites.get(e), projection_2D);
//    }
//
//    // DRAW LEVEL
//    for(Entity e : registry.levelgeoms.entities)
//    {
//        drawSprite(registry.transforms.get(e), registry.sprites.get(e), projection_2D);
//    }

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