#include <fstream>
#include <iostream>
#include <cstdlib>
#include <json.hpp>

#include "common.hpp"
#include "tiny_ecs_registry.hpp"

#define TILE_SIZE 16
#define NUMROOMSWIDE 5
#define NUMFLOORS 5
#define NUMTILESWIDE NUMROOMSWIDE * 11
#define NUMTILESTALL NUMFLOORS * 9

INTERNAL void AddTileSizedCollider(Entity tileEntity)
{
    auto& collider = registry.colliders.emplace(tileEntity);
    collider.collision_neg = {0.f,0.f};
    collider.collision_pos = { TILE_SIZE, TILE_SIZE };
}

INTERNAL Entity CreateBasicLevelTile(i32 column, i32 row, TEXTURE_ASSET_ID texId = TEXTURE_ASSET_ID::MIDTILE1)
{
    Entity entity = Entity::CreateEntity(TAG_PLAYERBLOCKABLE);

    auto& transform = registry.transforms.emplace(entity);

    transform.position = vec2(column * TILE_SIZE, row * TILE_SIZE);
    transform.center = {0.f,0.f};

    registry.sprites.insert(
            entity,
            {
                { TILE_SIZE, TILE_SIZE },
                10,
                texId,
                EFFECT_ASSET_ID::SPRITE
            }
    );


    return entity;
}

INTERNAL Entity CreateLadderTile(i32 column, i32 row)
{
    Entity entity = Entity::CreateEntity(TAG_LADDER);

    auto& transform = registry.transforms.emplace(entity);

    transform.position = vec2(column * TILE_SIZE, row * TILE_SIZE);
    transform.center = {0.f,0.f};

    registry.sprites.insert(
            entity,
            {
                { TILE_SIZE, TILE_SIZE },
                0,
                TEXTURE_ASSET_ID::LADDER,
                EFFECT_ASSET_ID::SPRITE
            }
    );

    AddTileSizedCollider(entity);

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

INTERNAL void LoadAllLevelData()
{
    using JSON = nlohmann::json;

    std::ifstream ifs = std::ifstream(level_path("chapter1.json"));
    if(!ifs.is_open()){
        std::cerr <<" Failed to open level data..."<<std::endl;
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
        if(chapterOneRooms.count(roomRawData.type) == 0)
        {
            chapterOneRooms.insert({roomRawData.type, std::vector<ns::RoomRawData>()});
        }
        chapterOneRooms.at(roomRawData.type).push_back(roomRawData);
    }
}

struct CurrentLevelData
{
    vec2 playerStart;
    vec2 cameraBoundMin;
    vec2 cameraBoundMax;
};
INTERNAL CurrentLevelData currentLevelData;

INTERNAL Entity levelTiles[NUMTILESWIDE][NUMTILESTALL];

INTERNAL void ParseRoomData(const ns::RoomRawData& r, int roomXIndex, int roomYIndex)
{
    for(int i = 0; i < r.height; ++i)
    {
        for(int j = 0; j < r.width; ++j)
        {
            const char& c = r.data.at(i * r.width + j);
            switch(c)
            {
                case 'A':{
                    Entity tile = CreateBasicLevelTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                    levelTiles[roomXIndex * r.width + j][roomYIndex * r.height + i] = tile;
                }break;

                case '1':{
                    currentLevelData.playerStart = { roomXIndex*r.width*TILE_SIZE + j*TILE_SIZE + (TILE_SIZE/2.f) ,
                                                     roomYIndex*r.height*TILE_SIZE + i*TILE_SIZE + (TILE_SIZE/2.f) };
                }break;

                case '2':{
                    // end point
                }break;

                case 'L':{
                    // ladder
                    CreateLadderTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                }break;

                case 'B':{
                    Entity tile = CreateBasicLevelTile(roomXIndex * r.width + j, roomYIndex * r.height + i, TEXTURE_ASSET_ID::SKULLS1);
                    levelTiles[roomXIndex * r.width + j][roomYIndex * r.height + i] = tile;
                }break;

                default:{
                }break;
            }
        }
    }
}

INTERNAL void ChangeSpritesBasedOnTopBottom(Entity e, i32 col, i32 row)
{
    auto& spr = registry.sprites.get(e);
    if(spr.texId != TEXTURE_ASSET_ID::MIDTILE1)
    {
        return; 
    }

    bool topClear = row - 1 >= 0 && levelTiles[col][row - 1] == 0;
    bool botClear = row + 1 < NUMTILESTALL && levelTiles[col][row + 1] == 0;
    if (topClear && botClear)
    {
        spr.texId = TEXTURE_ASSET_ID::TOPBOTTILE1;
        spr.dimensions.y += 3;
        registry.transforms.get(e).center.y += 2;
    }
    else if(botClear)
    {
        spr.texId = TEXTURE_ASSET_ID::BOTTILE1;
        spr.dimensions.y = spr.dimensions.y + 1;
    }
    else if(topClear)
    {
        spr.texId = TEXTURE_ASSET_ID::TOPTILE1;
        spr.dimensions.y += 2;
        registry.transforms.get(e).center.y += 2;
    }
    else
    {
        // mid
        int which = rand()%10;
        switch(which)
        {
            case 0:{
                spr.texId = TEXTURE_ASSET_ID::SKULLS1;
            }break;
            case 1:{
                spr.texId = TEXTURE_ASSET_ID::SKULLS2;
            }break;
            default:{
            }break;
        }
    }
}

INTERNAL void AddColliderIfRequired(Entity tileEntity, i32 col, i32 row)
{
    bool topClear = row - 1 >= 0 && levelTiles[col][row - 1] == 0;
    bool botClear = row + 1 < NUMTILESTALL && levelTiles[col][row + 1] == 0;
    bool leftClear = col - 1 >= 0 && levelTiles[col - 1][row] == 0;
    bool rightClear = col + 1 < NUMTILESWIDE && levelTiles[col + 1][row] == 0;
    bool atLeastOneFaceIsClear = topClear || botClear || leftClear || rightClear;
    if(atLeastOneFaceIsClear)
    {
        AddTileSizedCollider(tileEntity);
    }
}

/** Process and ready the level for gameplay.
 *  Change sprites for top or bottom tiles.
 *  Add colliders to tiles that can be collided with. */
INTERNAL void UpdateLevelGeometry()
{
    for(int col = 0; col < NUMTILESWIDE; ++col)
    {
        for(int row = 0; row < NUMTILESTALL; ++row)
        {
            Entity e = levelTiles[col][row];
            if(e != 0)
            {
                ChangeSpritesBasedOnTopBottom(e, col, row);
                AddColliderIfRequired(e, col, row);
            }
        }
    }
}

INTERNAL void GenerateNewLevel()
{
    std::array<std::array<ns::RoomRawData, NUMROOMSWIDE>, NUMFLOORS> roomDataArray;
    const ns::RoomRawData& sampleRoom = chapterOneRooms.at("start")[0];
    i32 rw = sampleRoom.width;
    i32 rh = sampleRoom.height;
    char* tileDataArray = (char*) malloc(rw * rh);

    // FALLING AND LANDING ROOMS
    for(size_t floor = 0; floor < NUMFLOORS-1; ++floor)
    {
        u32 fallingAndLanding = rand() % NUMROOMSWIDE;
        while(!roomDataArray[floor][fallingAndLanding].data.empty()
           || !roomDataArray[floor + 1][fallingAndLanding].data.empty())
        {
            fallingAndLanding = rand() % NUMROOMSWIDE;
        }
        u32 fallingRoomIndex = rand() % chapterOneRooms["falling"].size();
        roomDataArray[floor][fallingAndLanding] = chapterOneRooms["falling"][fallingRoomIndex];

        u32 landingRoomIndex = rand() % chapterOneRooms["landing"].size();
        roomDataArray[floor + 1][fallingAndLanding] = chapterOneRooms["landing"][landingRoomIndex];
    }

    // END ROOM
    u32 end = rand() % NUMROOMSWIDE;
    while(!roomDataArray[0][end].data.empty())
    {
        end = rand() % NUMROOMSWIDE;
    }
    u32 endRoomIndex = rand() % chapterOneRooms["end"].size();
    roomDataArray[0][end] = chapterOneRooms["end"][endRoomIndex];

    // START ROOM
    u32 start = rand() % NUMROOMSWIDE;
    while(!roomDataArray[NUMFLOORS-1][start].data.empty())
    {
        start = rand() % NUMROOMSWIDE;
    }
    u32 startRoomIndex = rand() % chapterOneRooms["start"].size();
    roomDataArray[NUMFLOORS-1][start] = chapterOneRooms["start"][startRoomIndex];

    // CORRIDORS
    for(int i = 0; i < NUMFLOORS; ++i)
    {
        for(int j = 0; j < NUMROOMSWIDE; ++j)
        {
            if(roomDataArray[i][j].data.empty())
            {
                u32 corridorRoomIndex = rand() % chapterOneRooms["corridor"].size();
                roomDataArray[i][j] = chapterOneRooms["corridor"][corridorRoomIndex];
            }
        }
    }

    // actually create tiles
    for(int i = 0; i < NUMFLOORS; ++i)
    {
        for(int j = 0; j < NUMROOMSWIDE; ++j)
        {
            ParseRoomData(roomDataArray[i][j], j, i);
        }
    }

    // Process and prepare the level
    UpdateLevelGeometry();

    // Boundary
    for(int i = -1; i < ((NUMTILESWIDE)+1); ++i)
    {
        auto _a = CreateBasicLevelTile(i, -1);
        auto _b = CreateBasicLevelTile(i, NUMTILESTALL);
        AddTileSizedCollider(_a);
        AddTileSizedCollider(_b);
    }
    for(int i = -1; i < ((NUMTILESTALL)+1); ++i)
    {
        auto _a = CreateBasicLevelTile(-1, i);
        auto _b = CreateBasicLevelTile(NUMTILESWIDE, i);
        AddTileSizedCollider(_a);
        AddTileSizedCollider(_b);
    }

    float halfWidth = (float) GAME_RESOLUTION_WIDTH / 2.f;
    float halfHeight = (float) GAME_RESOLUTION_HEIGHT / 2.f;

    currentLevelData.cameraBoundMin.x = (-1 * TILE_SIZE) + halfWidth;
    currentLevelData.cameraBoundMin.y = (-1 * TILE_SIZE) + halfHeight;
    currentLevelData.cameraBoundMax.x = (((NUMTILESWIDE)+1) * TILE_SIZE) - halfWidth;
    currentLevelData.cameraBoundMax.y = (((NUMTILESTALL)+1) * TILE_SIZE) - halfHeight;

    free(tileDataArray);
}

void SetRandomizerSeed(u32 seed)
{
    srand(seed);
}
