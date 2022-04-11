#include <fstream>
#include <iostream>
#include <cstdlib>
#include <json.hpp>

#include "common.hpp"
#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

INTERNAL void AddTileSizedCollider(Entity tileEntity)
{
    auto& transform = registry.transforms.get(tileEntity);
    auto& collider = registry.colliders.emplace(tileEntity);
    collider.collision_neg = { 0, 0 };
    collider.collision_pos = { TILE_SIZE, TILE_SIZE };
    collider.collider_position = transform.position;
}

INTERNAL Entity CreateBasicLevelTile(i32 column, i32 row, u16 spriteFrame = 0)
{
    Entity entity = Entity::CreateEntity(TAG_PLAYERBLOCKABLE);

    auto& transform = registry.transforms.emplace(entity);
    transform.position = vec2(column * TILE_SIZE, row * TILE_SIZE);
    transform.center = { 0.f,0.f };

    registry.sprites.insert(
        entity,
        {
            { TILE_SIZE, TILE_SIZE },
            10,
            EFFECT_ASSET_ID::SPRITE,
            TEXTURE_ASSET_ID::TILES_CAVE,
            true, false, true, 80, 128,
            0,
            0,
            0.f,
            {
                {
                    1,
                    spriteFrame,
                    0.f
                }
            }
        }
    );

    return entity;
}

INTERNAL Entity CreateLadderTile(i32 column, i32 row)
{
    Entity entity = Entity::CreateEntity(TAG_LADDER);

    auto& transform = registry.transforms.emplace(entity);
    transform.position = vec2(column * TILE_SIZE, row * TILE_SIZE);
    transform.center = { 0.f,0.f };

    registry.sprites.insert(
        entity,
        {
            { TILE_SIZE, TILE_SIZE },
            0,
            EFFECT_ASSET_ID::SPRITE,
            TEXTURE_ASSET_ID::ASCENT_LEVELTILES_SHEET,
            true, false, true, 256, 256,
            0,
            0,
            0.f,
            {
                {
                    1,
                    10,
                    0.f
                }
            }
        }
    );

    AddTileSizedCollider(entity);

    return entity;
}

INTERNAL Entity CreateSpikeTile(i32 col, i32 row)
{
    Entity entity = Entity::CreateEntity(TAG_SPIKE);

    auto& transform = registry.transforms.emplace(entity);
    transform.center = { 1.f,8.f };
    transform.position = vec2(col * TILE_SIZE + transform.center.x,
        row * TILE_SIZE + transform.center.y);

    u8 whichSpike = rand() % 2;
    u16 spikeTexFrame = whichSpike ? 7 : 8;
    registry.sprites.insert(
        entity,
        {
            { TILE_SIZE, TILE_SIZE },
            0,
            EFFECT_ASSET_ID::SPRITE,
            TEXTURE_ASSET_ID::ASCENT_LEVELTILES_SHEET,
            true, false, true, 256, 256,
            0,
            0,
            0.f,
            {
                {
                    1,
                    spikeTexFrame,
                    0.f
                }
            }
        }
    );

    auto& collider = registry.colliders.emplace(entity); // TODO
    collider.collider_position = transform.position;
    collider.collision_neg = { 0, 0 };
    collider.collision_pos = { 6, 8 };

    return entity;
}

INTERNAL Entity CreateEndPointTile(i32 col, i32 row)
{
    Entity entity = Entity::CreateEntity(TAG_LEVELENDPOINT);

    auto& transform = registry.transforms.emplace(entity);
    transform.position = vec2(col * TILE_SIZE, row * TILE_SIZE);
    transform.center = { 0.f,0.f };

    registry.sprites.insert(
        entity,
        {
            { TILE_SIZE, TILE_SIZE },
            0,
            EFFECT_ASSET_ID::SPRITE,
            TEXTURE_ASSET_ID::ASCENT_LEVELTILES_SHEET,
            true, false, true, 256, 256,
            0,
            0,
            0.f,
            {
                {
                    1,
                    9,
                    0.f
                }
            }
        }
    );

    AddTileSizedCollider(entity);

    ProximityTextComponent& newText = registry.proximityTexts.emplace(entity);
    newText.triggerPosition = transform.position + vec2(TILE_SIZE/2.f, TILE_SIZE/2.f);
    newText.triggerRadius = 12.f;
    newText.textPosition = newText.triggerPosition + vec2(0.f, -9.f);
    newText.textSize = 8;
    newText.text = std::string("Press UP to go to next stage");
    newText.bTyped = true;
    newText.secondsBetweenTypedCharacters = 0.025f;

    return entity;
}

INTERNAL Entity CreateShopBackground(i32 col, i32 row)
{
    Entity entity = Entity::CreateEntity();

    auto& transform = registry.transforms.emplace(entity);
    transform.position = vec2(col * TILE_SIZE, row * TILE_SIZE);
    transform.center = { 0.f,0.f };

    registry.sprites.insert(
        entity,
        {
            { TILE_SIZE * ROOM_DIMENSION_X , TILE_SIZE * ROOM_DIMENSION_Y},
            -128,
            EFFECT_ASSET_ID::SPRITE,
            TEXTURE_ASSET_ID::SHOPBG
        }
    );

    return entity;
}

namespace ns
{
    using JSON = nlohmann::json;

    struct RoomRawData
    {
        std::string name;
        std::string type;
        std::string data;
        int width = -1;
        int height = -1;
    };

    INTERNAL void from_json(const JSON& j, RoomRawData& roomRawData)
    {
        j.at("name").get_to(roomRawData.name);
        j.at("type").get_to(roomRawData.type);
        j.at("data").get_to(roomRawData.data);
        j.at("width").get_to(roomRawData.width);
        j.at("height").get_to(roomRawData.height);
    }
}

//                          ROOM TYPE    ARRAY OF ROOM DATA
INTERNAL std::unordered_map<std::string, std::vector<ns::RoomRawData>> chapterOneRooms;
INTERNAL std::unordered_map<std::string, std::vector<ns::RoomRawData>> chapterTwoRooms;
INTERNAL std::unordered_map<std::string, std::vector<ns::RoomRawData>> chapterThreeRooms;
INTERNAL ns::RoomRawData tutorialRoomData;

INTERNAL void LoadAllLevelData()
{
    using JSON = nlohmann::json;

    std::ifstream ifs = std::ifstream(level_path("chapter1.json"));
    if (!ifs.is_open()) {
        std::cerr << " Failed to open level data..." << std::endl;
        assert(0);
        return;
    }
    JSON roomData;
    ifs >> roomData;
    ifs.close();
    int roomCount = roomData["count"].get<int>();
    JSON roomDataJsonArray = roomData["rooms"];
    for (JSON::iterator it = roomDataJsonArray.begin(); it != roomDataJsonArray.end(); ++it)
    {
        JSON room = *it;
        auto roomRawData = room.get<ns::RoomRawData>();
        if (chapterOneRooms.count(roomRawData.type) == 0)
        {
            chapterOneRooms.insert({ roomRawData.type, std::vector<ns::RoomRawData>() });
        }
        chapterOneRooms.at(roomRawData.type).push_back(roomRawData);
    }

    ifs = std::ifstream(level_path("chapter2.json"));
    if (!ifs.is_open()) {
        std::cerr << " Failed to open level data..." << std::endl;
        assert(0);
        return;
    }
    ifs >> roomData;
    ifs.close();
    roomCount = roomData["count"].get<int>();
    roomDataJsonArray = roomData["rooms"];
    for (JSON::iterator it = roomDataJsonArray.begin(); it != roomDataJsonArray.end(); ++it)
    {
        JSON room = *it;
        auto roomRawData = room.get<ns::RoomRawData>();
        if (chapterTwoRooms.count(roomRawData.type) == 0)
        {
            chapterTwoRooms.insert({ roomRawData.type, std::vector<ns::RoomRawData>() });
        }
        chapterTwoRooms.at(roomRawData.type).push_back(roomRawData);
    }

    ifs = std::ifstream(level_path("chapter3.json"));
    if (!ifs.is_open()) {
        std::cerr << " Failed to open level data..." << std::endl;
        assert(0);
        return;
    }
    ifs >> roomData;
    ifs.close();
    roomCount = roomData["count"].get<int>();
    roomDataJsonArray = roomData["rooms"];
    for (JSON::iterator it = roomDataJsonArray.begin(); it != roomDataJsonArray.end(); ++it)
    {
        JSON room = *it;
        auto roomRawData = room.get<ns::RoomRawData>();
        if (chapterThreeRooms.count(roomRawData.type) == 0)
        {
            chapterThreeRooms.insert({ roomRawData.type, std::vector<ns::RoomRawData>() });
        }
        chapterThreeRooms.at(roomRawData.type).push_back(roomRawData);
    }

    ifs = std::ifstream(level_path("tutorial.json"));
    if (!ifs.is_open()) {
        std::cerr << " Failed to open level data..." << std::endl;
        assert(0);
        return;
    }
    ifs >> roomData;
    ifs.close();
    roomDataJsonArray = roomData["rooms"];
    JSON room = *roomDataJsonArray.begin();
    auto roomRawData = room.get<ns::RoomRawData>();
    tutorialRoomData = roomRawData;
}

struct CurrentLevelData
{
    vec2 cameraBoundMin;
    vec2 cameraBoundMax;

    vec2 playerStart;
    std::vector<vec2> groundMonsterSpawns;
    std::vector<vec2> flyingMonsterSpawns;
    std::vector<vec2> treasureSpawns;
    std::vector<vec2> shopItemSpawns;
};
INTERNAL CurrentLevelData currentLevelData;

INTERNAL std::vector<std::vector<Entity>> levelTiles;

INTERNAL void ClearCurrentLevelData()
{
    currentLevelData.playerStart = vec2(0.f, 0.f);
    currentLevelData.groundMonsterSpawns.clear();
    currentLevelData.flyingMonsterSpawns.clear();
    currentLevelData.treasureSpawns.clear();
    currentLevelData.shopItemSpawns.clear();
}

INTERNAL void ClearLevelTiles()
{
    for (int i = 0; i < levelTiles.size(); ++i)
    {
        for (int j = 0; j < levelTiles[0].size(); ++j)
        {
            levelTiles[i][j] = Entity();
        }
    }
}

INTERNAL void ParseRoomData(const ns::RoomRawData r, int roomXIndex, int roomYIndex)
{
    // Check room type
    if (r.type == "shop")
    {
        CreateShopBackground(roomXIndex * r.width, roomYIndex * r.height);
    }

    // Check individual tiles
    for (int i = 0; i < r.height; ++i)
    {
        for (int j = 0; j < r.width; ++j)
        {
            const char c = r.data.at(i * r.width + j);
            switch (c)
            {
                case 'A': {
                    Entity tile = CreateBasicLevelTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                    levelTiles[roomXIndex * r.width + j][roomYIndex * r.height + i] = tile;
                }break;
                case 'C': {
                    u8 roll = rand() % 2;
                    if (roll == 0)
                    {
                        Entity tile = CreateBasicLevelTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                        levelTiles[roomXIndex * r.width + j][roomYIndex * r.height + i] = tile;
                    }
                }break;

                case '1': {
                    currentLevelData.playerStart = { roomXIndex * r.width * TILE_SIZE + j * TILE_SIZE + (TILE_SIZE / 2.f),
                                                     roomYIndex * r.height * TILE_SIZE + i * TILE_SIZE + (TILE_SIZE / 2.f) };
                }break;

                case 'M': {
                    currentLevelData.groundMonsterSpawns.push_back({ roomXIndex * r.width * TILE_SIZE + j * TILE_SIZE + (TILE_SIZE / 2.f),
                                                     roomYIndex * r.height * TILE_SIZE + i * TILE_SIZE + (TILE_SIZE / 2.f) });
                }break;
                case 'N': {
                    currentLevelData.flyingMonsterSpawns.push_back({ roomXIndex * r.width * TILE_SIZE + j * TILE_SIZE + (TILE_SIZE / 2.f),
                                                     roomYIndex * r.height * TILE_SIZE + i * TILE_SIZE + (TILE_SIZE / 2.f) });
                }break;

                case 'T': {
                    currentLevelData.treasureSpawns.push_back({ roomXIndex * r.width * TILE_SIZE + j * TILE_SIZE + (TILE_SIZE / 2.f),
                                                     roomYIndex * r.height * TILE_SIZE + i * TILE_SIZE + (TILE_SIZE / 2.f) });
                }break;

                case 'S': {
                    // shop items
                    currentLevelData.shopItemSpawns.push_back({ roomXIndex * r.width * TILE_SIZE + j * TILE_SIZE,
                                                     roomYIndex * r.height * TILE_SIZE + i * TILE_SIZE });
                }break;

                case 'R': {
                    CreateShopKeeperNPC({ roomXIndex * r.width * TILE_SIZE + j * TILE_SIZE + TILE_SIZE/2.f, 
                                          roomYIndex * r.height * TILE_SIZE + i * TILE_SIZE + TILE_SIZE/2.f});
                }break;

                case '2': {
                    // end point
                    CreateEndPointTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                }break;

                case 'L': {
                    // ladder
                    CreateLadderTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                }break;

                case 'W': {
                    // spikes
                    CreateSpikeTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                }break;

                case 'B': {
                    // wooden tiles
                    // Entity tile = CreateBasicLevelTile(roomXIndex * r.width + j, roomYIndex * r.height + i, 6);
                    // levelTiles[roomXIndex * r.width + j][roomYIndex * r.height + i] = tile;
                    Entity tile = CreateBasicLevelTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                    levelTiles[roomXIndex * r.width + j][roomYIndex * r.height + i] = tile;
                }break;

                default: {
                }break;
            }
        }
    }
}

INTERNAL void ChangeSpritesBasedOnTopBottom(Entity e, i32 col, i32 row)
{
    auto& spr = registry.sprites.get(e);
    if (spr.GetStartFrame() != 0)
    {
        return;
    }

    bool topClear = (row - 1 >= 0 && (col >= 0 && col < levelTiles.size())) && levelTiles[col][row - 1] == 0;
    bool botClear = (row + 1 < levelTiles[0].size() && (col >= 0 && col < levelTiles.size())) && levelTiles[col][row + 1] == 0;
    bool leftClear = (col - 1 >= 0 && (row >= 0 && row < levelTiles[0].size())) && levelTiles[col - 1][row] == 0;
    bool rightClear = (col + 1 < levelTiles.size() && (row >= 0 && row < levelTiles[0].size())) && levelTiles[col + 1][row] == 0;

    bool tlCornerClear = (row - 1 >= 0 && col - 1 >= 0) && levelTiles[col - 1][row - 1] == 0;
    bool trCornerClear = (row - 1 >= 0 && col + 1 < levelTiles.size()) && levelTiles[col + 1][row - 1] == 0;
    bool blCornerClear = (row + 1 < levelTiles[0].size() && col - 1 >= 0) && levelTiles[col - 1][row + 1] == 0;
    bool brCornerClear = (row + 1 < levelTiles[0].size() && col + 1 < levelTiles.size()) && levelTiles[col + 1][row + 1] == 0;

    if (topClear && botClear && leftClear && rightClear)
    {
        spr.SetStartFrame(7);
    }
    else if (topClear && botClear && leftClear)
    {
        spr.SetStartFrame(1);
    }
    else if (topClear && botClear && rightClear)
    {
        spr.SetStartFrame(3);
    }
    else if (topClear && leftClear && rightClear)
    {
        spr.SetStartFrame(6);
    }
    else if (botClear && leftClear && rightClear)
    {
        spr.SetStartFrame(8);
    }
    else if (topClear && botClear)
    {
        spr.SetStartFrame(14);
    }
    else if (leftClear && rightClear)
    {
        spr.SetStartFrame(10);
    }
    else if (botClear && rightClear)
    {
        spr.SetStartFrame(9);
    }
    else if (botClear && leftClear)
    {
        spr.SetStartFrame(5);
    }
    else if (topClear && leftClear)
    {
        spr.SetStartFrame(0);
    }
    else if (topClear && rightClear)
    {
        spr.SetStartFrame(4);
    }
    else if (topClear)
    {
        spr.SetStartFrame(2);
    }
    else if (botClear)
    {
        spr.SetStartFrame(12);
    }
    else if (leftClear)
    {
        spr.SetStartFrame(11);
    }
    else if (rightClear)
    {
        spr.SetStartFrame(13);
    }
    else
    {
        if(tlCornerClear)
        {
            spr.SetStartFrame(16);
        }
        else if(trCornerClear)
        {
            spr.SetStartFrame(17);
        }
        else if(blCornerClear)
        {
            spr.SetStartFrame(18);
        }
        else if(brCornerClear)
        {
            spr.SetStartFrame(19);
        }
        else
        {
            spr.SetStartFrame(15);
        }
        // // mid
        // int which = rand() % 10;
        // switch (which)
        // {
        //     case 0: {
        //         spr.SetStartFrame(4);
        //     }break;
        //     case 1: {
        //         spr.SetStartFrame(5);
        //     }break;
        //     default: {
        //         spr.SetStartFrame();
        //     }break;
        // }
    }

    // if(row == -1 && col == -1)
    // {
    //     spr.SetStartFrame(15);
    // }
    // if(row == -1 && col == levelTiles.size())
    // {
    //     spr.SetStartFrame(15);
    // }
    // if(row == levelTiles[0].size() && col == -1)
    // {
    //     spr.SetStartFrame(15);
    // }
    // if(row == levelTiles[0].size() && col == levelTiles.size())
    // {
    //     spr.SetStartFrame(15);
    // }
}

INTERNAL void AddColliderIfRequired(Entity tileEntity, i32 col, i32 row)
{
    bool topClear = row - 1 >= 0 && levelTiles[col][row - 1] == 0;
    bool botClear = row + 1 < levelTiles[0].size() && levelTiles[col][row + 1] == 0;
    bool leftClear = col - 1 >= 0 && levelTiles[col - 1][row] == 0;
    bool rightClear = col + 1 < levelTiles.size() && levelTiles[col + 1][row] == 0;
    bool atLeastOneFaceIsClear = topClear || botClear || leftClear || rightClear;
    if (atLeastOneFaceIsClear)
    {
        AddTileSizedCollider(tileEntity);
    }
}

/** Process and ready the level for gameplay.
 *  Change sprites for top or bottom tiles.
 *  Add colliders to tiles that can be collided with. */
INTERNAL void UpdateLevelGeometry(GAMELEVELENUM stageToGenerate)
{
    switch(stageToGenerate)
    {
        case CHAPTER_TUTORIAL:
        {
            for (int col = 0; col < tutorialRoomData.width; ++col)
            {
                for (int row = 0; row < tutorialRoomData.height; ++row)
                {
                    Entity e = levelTiles[col][row];
                    if (e != 0)
                    {
                        ChangeSpritesBasedOnTopBottom(e, col, row);
                        AddColliderIfRequired(e, col, row);
                    }
                }
            }
            break;
        }

        case CHAPTER_ONE_STAGE_ONE:
        {
            for (int col = 0; col < NUMTILESWIDE; ++col)
            {
                for (int row = 0; row < NUMTILESTALL; ++row)
                {
                    Entity e = levelTiles[col][row];
                    if (e != 0)
                    {
                        ChangeSpritesBasedOnTopBottom(e, col, row);
                        AddColliderIfRequired(e, col, row);
                    }
                }
            }
            break;
        }

        case CHAPTER_TWO_STAGE_ONE:
        {
            for (int col = 0; col < NUMTILESWIDE; ++col)
            {
                for (int row = 0; row < NUMTILESTALL; ++row)
                {
                    Entity e = levelTiles[col][row];
                    if (e != 0)
                    {
                        ChangeSpritesBasedOnTopBottom(e, col, row);
                        AddColliderIfRequired(e, col, row);
                    }
                }
            }
            break;
        }

        case CHAPTER_THREE_STAGE_ONE:
        {
            for (int col = 0; col < NUMTILESWIDE; ++col)
            {
                for (int row = 0; row < NUMTILESTALL; ++row)
                {
                    Entity e = levelTiles[col][row];
                    if (e != 0)
                    {
                        ChangeSpritesBasedOnTopBottom(e, col, row);
                        AddColliderIfRequired(e, col, row);
                    }
                }
            }
            break;
        }
    }
    
}

INTERNAL void GenerateRooms(std::unordered_map<std::string, std::vector<ns::RoomRawData>> currentChapterRooms)
{
    std::array<std::array<ns::RoomRawData, NUMROOMSWIDE>, NUMFLOORS> roomDataArray;
    const ns::RoomRawData& sampleRoom = currentChapterRooms.at("start")[0];
    i32 rw = sampleRoom.width;
    i32 rh = sampleRoom.height;
    char* tileDataArray = (char*)malloc(rw * rh);

    // FALLING AND LANDING ROOMS
    for (size_t floor = 0; floor < NUMFLOORS - 1; ++floor)
    {
        u32 fallingAndLanding = rand() % NUMROOMSWIDE;
        while (!roomDataArray[floor][fallingAndLanding].data.empty()
            || !roomDataArray[floor + 1][fallingAndLanding].data.empty())
        {
            fallingAndLanding = rand() % NUMROOMSWIDE;
        }
        u32 fallingRoomIndex = rand() % currentChapterRooms["falling"].size();
        roomDataArray[floor][fallingAndLanding] = currentChapterRooms["falling"][fallingRoomIndex];

        u32 landingRoomIndex = rand() % currentChapterRooms["landing"].size();
        roomDataArray[floor + 1][fallingAndLanding] = currentChapterRooms["landing"][landingRoomIndex];
    }

    // END ROOM
    u32 end = rand() % NUMROOMSWIDE;
    while (!roomDataArray[0][end].data.empty())
    {
        end = rand() % NUMROOMSWIDE;
    }
    u32 endRoomIndex = rand() % currentChapterRooms["end"].size();
    roomDataArray[0][end] = currentChapterRooms["end"][endRoomIndex];

    // START ROOM
    u32 start = rand() % NUMROOMSWIDE;
    while (!roomDataArray[NUMFLOORS - 1][start].data.empty())
    {
        start = rand() % NUMROOMSWIDE;
    }
    u32 startRoomIndex = rand() % currentChapterRooms["start"].size();
    roomDataArray[NUMFLOORS - 1][start] = currentChapterRooms["start"][startRoomIndex];

    // SHOP ROOM
    u32 shopCol = rand() % 2 == 0 ? 0 : NUMROOMSWIDE - 1;
    u32 shopRow = 1 + rand() % (NUMFLOORS - 2);
    u32 loop = 0;
    while (!roomDataArray[shopRow][shopCol].data.empty())
    {
        if (loop > 10)
        {
            shopCol = rand() % NUMROOMSWIDE;
            shopRow = 1 + rand() % (NUMFLOORS - 2);
        }
        else
        {
            shopCol = rand() % 2 == 0 ? 0 : NUMROOMSWIDE - 1;
            shopRow = 1 + rand() % (NUMFLOORS - 2);
        }
        ++loop;
    }
    u32 shopRoomIndex = rand() % currentChapterRooms["shop"].size();
    roomDataArray[shopRow][shopCol] = currentChapterRooms["shop"][shopRoomIndex];

    // CORRIDORS
    for (int i = 0; i < NUMFLOORS; ++i)
    {
        for (int j = 0; j < NUMROOMSWIDE; ++j)
        {
            if (roomDataArray[i][j].data.empty())
            {
                u32 corridorRoomIndex = rand() % currentChapterRooms["corridor"].size();
                roomDataArray[i][j] = currentChapterRooms["corridor"][corridorRoomIndex];
            }
        }
    }

    // actually create tiles
    for (int i = 0; i < NUMFLOORS; ++i)
    {
        for (int j = 0; j < NUMROOMSWIDE; ++j)
        {
            ParseRoomData(roomDataArray[i][j], j, i);
        }
    }

    free(tileDataArray);
}

INTERNAL void GenerateNewLevel(GAMELEVELENUM stageToGenerate)
{
    ClearLevelTiles();
    ClearCurrentLevelData();

    std::unordered_map<std::string, std::vector<ns::RoomRawData>> currentChapterRooms;
    switch(stageToGenerate)
    {
        case CHAPTER_ONE_STAGE_ONE:{
            currentChapterRooms = chapterOneRooms;
        }break;
        case CHAPTER_TWO_STAGE_ONE:{
            currentChapterRooms = chapterTwoRooms;
        }break;
        case CHAPTER_THREE_STAGE_ONE:{
            currentChapterRooms = chapterThreeRooms;
        }break;
    }
    switch(stageToGenerate)
    {
        case CHAPTER_TUTORIAL:
        {
            levelTiles.resize(tutorialRoomData.width);
            for(auto& ltv : levelTiles)
            {
                ltv.resize(tutorialRoomData.height);
            }
        }break;

        case CHAPTER_ONE_STAGE_ONE:
        case CHAPTER_TWO_STAGE_ONE:
        case CHAPTER_THREE_STAGE_ONE:
        {
            levelTiles.resize(NUMTILESWIDE);
            for(auto& ltv : levelTiles)
            {
                ltv.resize(NUMTILESTALL);
            }
        }break;
    }

    if(stageToGenerate != CHAPTER_TUTORIAL)
    {
        GenerateRooms(currentChapterRooms);
    }
    else
    {
        // tutorial
        ParseRoomData(tutorialRoomData, 0, 0);
    }

    // Process and prepare the level
    UpdateLevelGeometry(stageToGenerate);

    if(stageToGenerate == CHAPTER_ONE_STAGE_ONE)
    {
        // Boundary
        for (int i = -1; i < ((NUMTILESWIDE)+1); ++i)
        {
            auto _a = CreateBasicLevelTile(i, -1);
            auto _b = CreateBasicLevelTile(i, NUMTILESTALL);
            AddTileSizedCollider(_a);
            AddTileSizedCollider(_b);
            ChangeSpritesBasedOnTopBottom(_a, i, -1);
            ChangeSpritesBasedOnTopBottom(_b, i, NUMTILESTALL);
        }
        for (int i = -1; i < ((NUMTILESTALL)+1); ++i)
        {
            auto _a = CreateBasicLevelTile(-1, i);
            auto _b = CreateBasicLevelTile(NUMTILESWIDE, i);
            AddTileSizedCollider(_a);
            AddTileSizedCollider(_b);
            ChangeSpritesBasedOnTopBottom(_a, -1, i);
            ChangeSpritesBasedOnTopBottom(_b, NUMTILESWIDE, i);
        }
    }
    
    float halfWidth = (float)GAME_RESOLUTION_WIDTH / 2.f;
    float halfHeight = (float)GAME_RESOLUTION_HEIGHT / 2.f;

    int minx = (-1 * TILE_SIZE);
    int miny = (-1 * TILE_SIZE);
    int maxx = (int)levelTiles.size()+1;
    int maxy = (int)levelTiles[0].size()+1;
    if(stageToGenerate == CHAPTER_TUTORIAL)
    {
        minx = 0;
        miny = 0;
        maxx = (int)levelTiles.size();
        maxy = (int)levelTiles[0].size();
    }
    currentLevelData.cameraBoundMin.x = minx + halfWidth;
    currentLevelData.cameraBoundMin.y = miny + halfHeight;
    currentLevelData.cameraBoundMax.x = (maxx * TILE_SIZE) - halfWidth;
    currentLevelData.cameraBoundMax.y = (maxy * TILE_SIZE) - halfHeight;
}

void SetRandomizerSeed(u32 seed)
{
    srand(seed);
}