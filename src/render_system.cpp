// internal
#include <chrono>
#include "render_system.hpp"
#include "world_system.hpp"
#include "console.hpp"
#include "vertext.h"

#include "tiny_ecs_registry.hpp"

using Clock = std::chrono::high_resolution_clock;
INTERNAL u32 GetRenderState(const SpriteComponent& sprite)
{
    u32 state = 0;
    state |= (u8) (sprite.layer + 128);
    state <<= 16;
    state |= (u16) sprite.texId;
    state <<= 8;
    state |= (u8) sprite.shaderId;

    return state;
};

INTERNAL u16 GetTexIDFromRenderState(u32 state)
{
    return (state & 0x00FFFF00) >> 8;
}

INTERNAL u8 GetShaderIDFromRenderState(u32 state)
{
    return (state & 0x000000FF);
}

INTERNAL bool SpriteTransformPairSorter(SpriteTransformPair const& lhs, SpriteTransformPair const& rhs) 
{
    return lhs.renderState < rhs.renderState;
}

void RenderSystem::DrawBackground(TEXTURE_ASSET_ID texId, float offset)
{
    if(texId == TEXTURE_ASSET_ID::TEXTURE_COUNT)
    {
        return;
    }

    const GLuint used_effect_enum = (GLuint) EFFECT_ASSET_ID::BACKGROUND;
    const GLuint program = (GLuint)effects[used_effect_enum];

    glUseProgram(program);
    gl_has_errors();

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    gl_has_errors();

    vec2 cameraPosition = { 0.f, 0.f };

    int lightSize = 0;
    float lightArray[50] = { };
    GAMELEVELENUM stage = world->GetCurrentStage();
    if (registry.players.size() > 0 && ((stage == CHAPTER_ONE_STAGE_ONE) || (stage == CHAPTER_TWO_STAGE_ONE)) && !world->gamePaused) {
        Entity player = registry.players.entities[0];
        TransformComponent& playerTransform = registry.transforms.get(player);
        float playerPositionX = clamp(playerTransform.position.x, cameraBoundMin.x, cameraBoundMax.x);
        float playerPositionY = clamp(playerTransform.position.y, cameraBoundMin.y, cameraBoundMax.y);
        cameraPosition = vec2(playerPositionX, playerPositionY);

        lightSize = registry.lightSources.size();
        for (int i = 0; i < registry.lightSources.size(); i++) {
            vec2 pos = registry.transforms.get(registry.lightSources.entities[i]).position;
            lightArray[(i * 2)] = pos.x;
            lightArray[(i * 2) + 1] = pos.y;
        }
    }

    GLuint lightNum_loc = glGetUniformLocation(currProgram, "lightSize");
    GLuint lightArray_loc = glGetUniformLocation(currProgram, "lightSources");
    GLuint camerapos_loc = glGetUniformLocation(currProgram, "cameraPosition");

    glUniform1i(lightNum_loc, lightSize);
    glUniform2fv(lightArray_loc, 25, lightArray);
    glUniform2fv(camerapos_loc, 1, (float*)&cameraPosition);

    GLuint offset_loc = glGetUniformLocation(currProgram, "bg_offset");
    glUniform1f(offset_loc, offset);

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
                -1.f, 1.f, 0.f, 0.f,
                1.f, 1.f, 1.f, 0.f,
                -1.f, -1.f, 0.f, 1.f,
                1.f, -1.f, 1.f, 1.f
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
    GLuint texture_id = texture_gl_handles[(GLuint)texId];
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Draw
    glBindVertexArray(bgQuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgQuadIBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    gl_has_errors();
}

void RenderSystem::DrawMainMenuBackground(float elapsed_ms) {
    elapsedTime += elapsed_ms / 1000.f;

    // TODO: Add a way to specify offset differences for background without manually doing it
    DrawBackground(bgTexId[0].texId, (elapsedTime / 800.0f) * 1.5f);
    DrawBackground(bgTexId[1].texId, (elapsedTime / 200.0f) * 1.5f);
    DrawBackground(bgTexId[2].texId, (elapsedTime / 150.0f) * 1.5f);
    DrawBackground(bgTexId[3].texId, (elapsedTime / 100.0f) * 1.5f);
    DrawBackground(bgTexId[4].texId, (elapsedTime / 50.0f) * 1.5f);
}

void RenderSystem::DrawAllBackgrounds(float elapsed_ms)
{
    float offset = 0.f;
    if (world->GetCurrentMode() == MODE_MAINMENU) 
    {
        DrawMainMenuBackground(elapsed_ms);
    }
    else 
    {
        if (registry.players.size() > 0 && bgTexId.size() > 1) {
            Entity player = registry.players.entities[0];
            TransformComponent& playerTransform = registry.transforms.get(player);
            float playerPositionX = clamp(playerTransform.position.x, cameraBoundMin.x, cameraBoundMax.x);

            playerPositionX = playerPositionX - (GAME_RESOLUTION_WIDTH / 2.0f);
            playerPositionX = playerPositionX / GAME_RESOLUTION_WIDTH;
            offset = playerPositionX / ((float)bgTexId.size());
            offset *= 0.5f; // Constant to slow down movement
        }

        for (int i = 0; i < bgTexId.size(); i++) {
            DrawBackground(bgTexId[i].texId, offset);
            offset += offset * bgTexId[i].speedFactor;
        }
    }
}

void RenderSystem::BatchDrawAllSprites(std::vector<SpriteTransformPair>& sortedSprites, const mat3 &projection)
{
    SpriteComponent flushAtEndSprite;
    flushAtEndSprite.texId = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    SpriteTransformPair flushAtEnd;
    flushAtEnd.spritePtr = &flushAtEndSprite;
    sortedSprites.push_back(flushAtEnd); // adding an invalid SpriteTransformPair to the end to flush everything at end

    LOCAL_PERSIST u32 spriteBatchVAO;
    LOCAL_PERSIST u32 spriteBatchVBO;
    LOCAL_PERSIST u32 spriteBatchIBO;
    if(!spriteBatchVAO)
    {
        glGenVertexArrays(1, &spriteBatchVAO);
        glBindVertexArray(spriteBatchVAO);
        glGenBuffers(1, &spriteBatchVBO);
        glBindBuffer(GL_ARRAY_BUFFER, spriteBatchVBO);
        glBufferData(GL_ARRAY_BUFFER, 4 * 16, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &spriteBatchIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteBatchIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * 6, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // CAMERA TRANSFORM
    cameraTransform = Transform();
    Entity player = registry.players.entities[0];
    TransformComponent& playerTransform = registry.transforms.get(player);
    float playerPositionX = clamp(playerTransform.position.x, cameraBoundMin.x, cameraBoundMax.x);
    float playerPositionY = clamp(playerTransform.position.y, cameraBoundMin.y, cameraBoundMax.y);
    vec2 cameraPosition = vec2(playerPositionX, playerPositionY);
    playerPositionX = playerPositionX - (GAME_RESOLUTION_WIDTH / 2.0f);
    playerPositionY = playerPositionY - (GAME_RESOLUTION_HEIGHT / 2.0f);
    vec2 playerPosition = vec2(playerPositionX, playerPositionY);
    vec2 scaledPlayerPosition = playerPosition * (float)FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
    cameraTransform.translate(-scaledPlayerPosition);

    int lightSize = 0;
    float lightArray[50] = { };
    if ((world->GetCurrentStage() == CHAPTER_ONE_STAGE_ONE) || (world->GetCurrentStage() == CHAPTER_TWO_STAGE_ONE)) {
        lightSize = registry.lightSources.size();
        for (int i = 0; i < registry.lightSources.size(); i++) {
            vec2 pos = registry.transforms.get(registry.lightSources.entities[i]).position;
            lightArray[(i * 2)] = pos.x;
            lightArray[(i * 2) + 1] = pos.y;
        }
    }

    // STD::VECTORS TO HOLD VERTICES AND INDICES BATCH
    u32 renderState = sortedSprites[0].renderState;
    std::vector<float> vertices(16 * sortedSprites.size());
    u32 verticesCount = 0;
    std::vector<u32> indices(6 * sortedSprites.size());
    u32 indicesCount = 0;

    for(auto& sortedSprite : sortedSprites)
    {
        if(renderState != sortedSprite.renderState)
        {
            // FLUSH BATCH

            const GLuint used_effect_enum = (GLuint) GetShaderIDFromRenderState(renderState);
            const GLuint program = (GLuint)effects[used_effect_enum];

            glUseProgram(program);
            gl_has_errors();

            // UNIFORMS
            GLint currProgram;
            glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
            Transform transform;
            GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
            glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*) &transform.mat);
            GLuint camera_loc = glGetUniformLocation(currProgram, "cameraTransform");
            glUniformMatrix3fv(camera_loc, 1, GL_FALSE, (float*) &cameraTransform.mat);
            GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
            glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*) &projection);
            GLuint fcolor_loc = glGetUniformLocation(currProgram, "fcolor");
            glUniform3f(fcolor_loc, 1.f, 1.f, 1.f);
            GLuint lightNum_loc = glGetUniformLocation(currProgram, "lightSize");
            GLuint lightArray_loc = glGetUniformLocation(currProgram, "lightSources");
            glUniform1i(lightNum_loc, lightSize);
            glUniform2fv(lightArray_loc, 25, lightArray);
            gl_has_errors();

            // REBIND VBO & IBO
            glBindVertexArray(spriteBatchVAO);
            glBindBuffer(GL_ARRAY_BUFFER, spriteBatchVBO);
            glBufferData(GL_ARRAY_BUFFER, 4 * verticesCount, vertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteBatchIBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indicesCount, indices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            // BIND THE TEXTURE FOR THIS BATCH
            glActiveTexture(GL_TEXTURE0);
            GLuint texture_id = texture_gl_handles[(GLuint)GetTexIDFromRenderState(renderState)];
            glBindTexture(GL_TEXTURE_2D, texture_id);

            // DRAW
            glBindVertexArray(spriteBatchVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteBatchIBO);
            glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            verticesCount = 0;
            indicesCount = 0;

            renderState = sortedSprite.renderState;
        }

        const SpriteComponent& sortedSpriteSprite = *(sortedSprite.spritePtr);

        vec2 scaledPosition = sortedSprite.transform.position * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
        vec2 topLeftCorner = scaledPosition - sortedSprite.transform.center * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
        vec2 scaledDimensions = sortedSprite.transform.scale * ((vec2) sortedSpriteSprite.dimensions) * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;

        vertices[verticesCount + 0] = topLeftCorner.x;
        vertices[verticesCount + 1] = topLeftCorner.y - 0.1f;
        vertices[verticesCount + 2] = 0.f; // U
        vertices[verticesCount + 3] = 0.f; // V
        vertices[verticesCount + 4] = topLeftCorner.x + scaledDimensions.x;
        vertices[verticesCount + 5] = topLeftCorner.y - 0.1f;
        vertices[verticesCount + 6] = 1.f; // U
        vertices[verticesCount + 7] = 0.f; // V
        vertices[verticesCount + 8] = topLeftCorner.x;
        vertices[verticesCount + 9] = topLeftCorner.y + scaledDimensions.y + 0.1f;
        vertices[verticesCount + 10] = 0.f; // U
        vertices[verticesCount + 11] = 1.f; // V
        vertices[verticesCount + 12] = topLeftCorner.x + scaledDimensions.x;
        vertices[verticesCount + 13] = topLeftCorner.y + scaledDimensions.y + 0.1f;
        vertices[verticesCount + 14] = 1.f; // U
        vertices[verticesCount + 15] = 1.f; // V

        if (sortedSpriteSprite.sprite_sheet) {

            size_t frame = sortedSpriteSprite.animations[sortedSpriteSprite.selected_animation].start_frame
                + sortedSpriteSprite.current_frame;

            size_t sheetX = (size_t) std::floor((float) sortedSpriteSprite.sheetSizeX / (float) sortedSpriteSprite.dimensions.x);
            size_t sheetY = (size_t) std::ceil((float) sortedSpriteSprite.sheetSizeY / (float) sortedSpriteSprite.dimensions.y);

            float offset_per_x = (1.f / (float) sheetX);
            float offset_per_y = (1.f / ((float) sortedSpriteSprite.sheetSizeY / (float) sortedSpriteSprite.dimensions.y));

            float offset_x = (float)(frame % sheetX);
            float offset_y = (float)(frame / sheetX);

            if (sortedSpriteSprite.reverse) {
                vertices[verticesCount + 6] = offset_x * offset_per_x; // U
                vertices[verticesCount + 7] = offset_y * offset_per_y; // V

                vertices[verticesCount + 2] = (offset_x + 1.0f) * offset_per_x; // U
                vertices[verticesCount + 3] = offset_y * offset_per_y; // V

                vertices[verticesCount + 14] = offset_x * offset_per_x; // U
                vertices[verticesCount + 15] = (offset_y + 1.0f) * offset_per_y; // V

                vertices[verticesCount + 10] = (offset_x + 1.0f) * offset_per_x; // U
                vertices[verticesCount + 11] = (offset_y + 1.0f) * offset_per_y; // V
            }
            else {
                vertices[verticesCount + 2] = offset_x * offset_per_x; // U
                vertices[verticesCount + 3] = offset_y * offset_per_y; // V

                vertices[verticesCount + 6] = (offset_x + 1.0f) * offset_per_x; // U
                vertices[verticesCount + 7] = offset_y * offset_per_y; // V

                vertices[verticesCount + 10] = offset_x * offset_per_x; // U
                vertices[verticesCount + 11] = (offset_y + 1.0f) * offset_per_y; // V

                vertices[verticesCount + 14] = (offset_x + 1.0f) * offset_per_x; // U
                vertices[verticesCount + 15] = (offset_y + 1.0f) * offset_per_y; // V
            }
        }

        if(sortedSprite.transform.rotation)
        {
            float c = cosf(sortedSprite.transform.rotation);
            float s = sinf(sortedSprite.transform.rotation);
            mat3 R = { { c, s, 0.f },{ -s, c, 0.f },{ 0.f, 0.f, 1.f } };

            vec2 tl = -sortedSprite.transform.center * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL;
            vec2 br = ((vec2)sortedSpriteSprite.dimensions) * (float) FRAMEBUFFER_PIXELS_PER_GAME_PIXEL + tl;
            vec2 tr = vec2(br.x, tl.y);
            vec2 bl = vec2(tl.x, br.y);

            tl = vec2(R * vec3(tl, 1.f));
            vertices[verticesCount + 0] = tl.x + scaledPosition.x;
            vertices[verticesCount + 1] = tl.y + scaledPosition.y - 0.1f;

            tr = vec2(R * vec3(tr, 1.f));
            vertices[verticesCount + 4] = tr.x + scaledPosition.x;
            vertices[verticesCount + 5] = tr.y + scaledPosition.y - 0.1f;

            bl = vec2(R * vec3(bl, 1.f));
            vertices[verticesCount + 8] = bl.x + scaledPosition.x;
            vertices[verticesCount + 9] = bl.y + scaledPosition.y + 0.1f;

            br = vec2(R * vec3(br, 1.f));
            vertices[verticesCount + 12] = br.x + scaledPosition.x;
            vertices[verticesCount + 13] = br.y + scaledPosition.y + 0.1f;
        }

        indices[indicesCount + 0] = 4*(indicesCount/6) + 0;
        indices[indicesCount + 1] = 4*(indicesCount/6) + 1;
        indices[indicesCount + 2] = 4*(indicesCount/6) + 3;
        indices[indicesCount + 3] = 4*(indicesCount/6) + 0;
        indices[indicesCount + 4] = 4*(indicesCount/6) + 3;
        indices[indicesCount + 5] = 4*(indicesCount/6) + 2;

        verticesCount += 16;
        indicesCount += 6;
    }
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::Draw(float elapsed_ms)
{
    // First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gameFrameBuffer);
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
	mat3 projection_2D = CreateGameProjectionMatrix();

    // DRAW BACKGROUND
    DrawAllBackgrounds(elapsed_ms);

    // DRAW SPRITES
    if(registry.sprites.size() > 0)
    {
        // SORTING FOR BATCH DRAWING
        std::vector<SpriteTransformPair> sortedSpriteArray(registry.sprites.size());
        for(size_t i = 0; i < registry.sprites.size(); ++i)
        {
            SpriteTransformPair s;
            s.spritePtr = &registry.sprites.components[i];
            s.renderState = GetRenderState(registry.sprites.components[i]);
            s.transform = registry.transforms.get(registry.sprites.entities[i]);
            sortedSpriteArray[i] = s;
        }

        // SORT
        std::sort(sortedSpriteArray.begin(), sortedSpriteArray.end(), &SpriteTransformPairSorter);

        // BATCH DRAW
        BatchDrawAllSprites(sortedSpriteArray, projection_2D);
    }

    // DRAW WORLD TEXT
    DrawWorldText(projection_2D);

    // DRAW UI
    DrawUI();

    FinalDrawToScreen(); // Truely render to the screen
}

void RenderSystem::DrawWorldText(const mat3& projection)
{
    vtxt_setflags(VTXT_CREATE_INDEX_BUFFER|VTXT_USE_CLIPSPACE_COORDS|VTXT_FLIP_Y);
    vtxt_backbuffersize(2, 2);
    for(WorldText& wt : worldTextsThisFrame)
    {
        vtxt_clear_buffer();
        vtxt_move_cursor(1, 1);
        vtxt_set_linegap_offset(-16.f);
        vtxt_append_line_centered(wt.text.c_str(), worldTextFontPtr, wt.size);
        vtxt_set_linegap_offset(0.f);
        vtxt_vertex_buffer vb = vtxt_grab_buffer();
        RebindMeshBufferObjects(worldTextVAO, vb.vertex_buffer, vb.index_buffer, vb.vertices_array_count, vb.indices_array_count);

        GLint currProgram;
        glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WORLDTEXT]);

        Transform worldTextTransform;
        worldTextTransform.translate(wt.pos*(float)FRAMEBUFFER_PIXELS_PER_GAME_PIXEL);
        worldTextTransform.scale(vec2(0.5f * (float)FRAMEBUFFER_PIXELS_PER_GAME_PIXEL, 0.5f * (float)FRAMEBUFFER_PIXELS_PER_GAME_PIXEL));

        glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
        GLuint projMatrix_loc = glGetUniformLocation(currProgram, "projectionMatrix");
        glUniformMatrix3fv(projMatrix_loc, 1, false, (float*)&projection);
        GLuint viewMatrix_loc = glGetUniformLocation(currProgram, "viewMatrix");
        glUniformMatrix3fv(viewMatrix_loc, 1, false, (float*)&cameraTransform.mat);
        GLuint modelMatrix_loc = glGetUniformLocation(currProgram, "modelMatrix");
        glUniformMatrix3fv(modelMatrix_loc, 1, false, (float*)&worldTextTransform.mat);

        glBindTexture(GL_TEXTURE_2D, worldTextFontAtlas.textureId);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(worldTextVAO.idVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, worldTextVAO.idIBO);
                glDrawElements(GL_TRIANGLES, worldTextVAO.indicesCount, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    worldTextsThisFrame.clear();
}

void RenderSystem::DrawUI()
{
    glBindFramebuffer(GL_FRAMEBUFFER, uiFrameBuffer);
    gl_has_errors();
    // Clearing backbuffer
    glViewport(0, 0, UI_LAYER_RESOLUTION_WIDTH, UI_LAYER_RESOLUTION_HEIGHT);
    glDepthRange(0.00001f, 10.f);
    glClearColor(0.674f, 0.847f, 1.0f, 0.0f);
    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    gl_has_errors();
    GLint currProgram;

    if(world->GetCurrentMode() == MODE_INGAME)
    {   
        glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::EXP_UI]);
        glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
        GLuint expProgress_loc = glGetUniformLocation(currProgram, "expProgress");
        glUniform1f(expProgress_loc, expProgressNormalized);
        glBindVertexArray(expProgressBar.idVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, expProgressBar.idIBO);
                glDrawElements(GL_TRIANGLES, expProgressBar.indicesCount, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::HEALTHBAR_BORDER_UI]);
        glBindVertexArray(healthBarBorder.idVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, healthBarBorder.idIBO);
                glDrawElements(GL_TRIANGLES, healthBarBorder.indicesCount, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::HEALTHBAR_UI]);
        glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
        GLuint hpNormalized_loc = glGetUniformLocation(currProgram, "hpNormalized");
        glUniform1f(hpNormalized_loc, healthPointsNormalized);
        glBindVertexArray(healthBar.idVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, healthBar.idIBO);
                glDrawElements(GL_TRIANGLES, healthBar.indicesCount, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if(showMutationSelect)
        {
            glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::MUTATION_SELECT_UI]);

            glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
            GLuint xOffset_loc = glGetUniformLocation(currProgram, "xOffset");
            GLuint bSelected_loc = glGetUniformLocation(currProgram, "bSelected");
            GLuint bBox_loc = glGetUniformLocation(currProgram, "bBox");
            GLuint time_loc = glGetUniformLocation(currProgram, "time");
            glUniform1f(time_loc, currentTimeInSeconds);
            glUniform1f(xOffset_loc, -0.6f);
            glUniform1i(bSelected_loc, false);
            glUniform1i(bBox_loc, true); 
            glBindVertexArray(mutationSelectBox.idVAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mutationSelectBox.idIBO);
                    glDrawElements(GL_TRIANGLES, mutationSelectBox.indicesCount, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glUniform1f(xOffset_loc, 0.f);
            glBindVertexArray(mutationSelectBox.idVAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mutationSelectBox.idIBO);
                    glDrawElements(GL_TRIANGLES, mutationSelectBox.indicesCount, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glUniform1f(xOffset_loc, 0.6f);
            glBindVertexArray(mutationSelectBox.idVAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mutationSelectBox.idIBO);
                    glDrawElements(GL_TRIANGLES, mutationSelectBox.indicesCount, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glUniform1f(xOffset_loc, -0.6f);
            glUniform1i(bSelected_loc, (mutationSelectionIndex == 0)); 
            glUniform1i(bBox_loc, false);
            glBindVertexArray(mutationSelectBorder.idVAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mutationSelectBorder.idIBO);
                    glDrawElements(GL_TRIANGLES, mutationSelectBorder.indicesCount, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glUniform1f(xOffset_loc, 0.0f);
            glUniform1i(bSelected_loc, (mutationSelectionIndex == 1)); 
            glBindVertexArray(mutationSelectBorder.idVAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mutationSelectBorder.idIBO);
                    glDrawElements(GL_TRIANGLES, mutationSelectBorder.indicesCount, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glUniform1f(xOffset_loc, 0.6f);
            glUniform1i(bSelected_loc, (mutationSelectionIndex == 2)); 
            glBindVertexArray(mutationSelectBorder.idVAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mutationSelectBorder.idIBO);
                    glDrawElements(GL_TRIANGLES, mutationSelectBorder.indicesCount, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        if (showShopSelect)
        {
            glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::MUTATION_SELECT_UI]);

            glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
            GLuint xOffset_loc = glGetUniformLocation(currProgram, "xOffset");
            GLuint bSelected_loc = glGetUniformLocation(currProgram, "bSelected");
            GLuint bBox_loc = glGetUniformLocation(currProgram, "bBox");
            GLuint time_loc = glGetUniformLocation(currProgram, "time");
            glUniform1f(time_loc, currentTimeInSeconds);
            glUniform1f(xOffset_loc, -0.6f);
            glUniform1i(bSelected_loc, false);
            glUniform1i(bBox_loc, true);

            glUniform1f(xOffset_loc, 0.f);
            glBindVertexArray(mutationSelectBox.idVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mutationSelectBox.idIBO);
            glDrawElements(GL_TRIANGLES, mutationSelectBox.indicesCount, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glUniform1i(bBox_loc, false);
            glUniform1f(xOffset_loc, 0.0f);
            glUniform1i(bSelected_loc, (mutationSelectionIndex == 1));
            glBindVertexArray(mutationSelectBorder.idVAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mutationSelectBorder.idIBO);
            glDrawElements(GL_TRIANGLES, mutationSelectBorder.indicesCount, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
    }

    glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::TEXT]);

    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    GLuint textColour_loc = glGetUniformLocation(currProgram, "textColour");
    glBindTexture(GL_TEXTURE_2D, textLayer1FontAtlas.textureId);
    glActiveTexture(GL_TEXTURE0);
    glUniform4f(textColour_loc, textLayer1Colour.x, textLayer1Colour.y, textLayer1Colour.z, textLayer1Colour.w);
    glBindVertexArray(textLayer1VAO.idVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textLayer1VAO.idIBO);
            glDrawElements(GL_TRIANGLES, textLayer1VAO.indicesCount, GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, textLayer2FontAtlas.textureId);
    glActiveTexture(GL_TEXTURE0);
    glUniform4f(textColour_loc, textLayer2Colour.x, textLayer2Colour.y, textLayer2Colour.z, textLayer2Colour.w);
    glBindVertexArray(textLayer2VAO.idVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textLayer2VAO.idIBO);
            glDrawElements(GL_TRIANGLES, textLayer2VAO.indicesCount, GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, textLayer3FontAtlas.textureId);
    glActiveTexture(GL_TEXTURE0);
    glUniform4f(textColour_loc, textLayer3Colour.x, textLayer3Colour.y, textLayer3Colour.z, textLayer3Colour.w);
    glBindVertexArray(textLayer3VAO.idVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textLayer3VAO.idIBO);
            glDrawElements(GL_TRIANGLES, textLayer3VAO.indicesCount, GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, textLayer4FontAtlas.textureId);
    glActiveTexture(GL_TEXTURE0);
    glUniform4f(textColour_loc, textLayer4Colour.x, textLayer4Colour.y, textLayer4Colour.z, textLayer4Colour.w);
    glBindVertexArray(textLayer4VAO.idVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textLayer4VAO.idIBO);
    glDrawElements(GL_TRIANGLES, textLayer4VAO.indicesCount, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);


    // CONSOLE RENDERING
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    console_render();
    glEnable(GL_DEPTH_TEST);
}

// draw the intermediate texture to the screen
void RenderSystem::FinalDrawToScreen()
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
        printf("Screen size changed.\n");
    }

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    GLuint darkenFactor_loc = glGetUniformLocation(currProgram, "darkenFactor");

    // Bind game frame texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, offScreenRenderBufferColor);
    world->darkenGameFrame ? glUniform1f(darkenFactor_loc, 0.5f) : glUniform1f(darkenFactor_loc, 0.0f);

    // Draw game frame
    glBindVertexArray(finalQuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Bind UI frame texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, offScreenUiBufferColor);
    glUniform1f(darkenFactor_loc, 0.0f);

    // Draw UI frame
    glBindVertexArray(finalQuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (world->gamePaused) {
        DrawBackground(TEXTURE_ASSET_ID::HELP_MENU, 0.f);
    }

    gl_has_errors();
}

mat3 RenderSystem::CreateGameProjectionMatrix()
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
