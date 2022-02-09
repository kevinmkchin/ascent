#include <fstream>
#include <iostream>
#include <cstdlib>
#include <json.hpp>

#include "common.hpp"
#include "tiny_ecs_registry.hpp"

#define TILE_SIZE 16

INTERNAL Entity CreateBasicLevelTile(i32 column, i32 row)
{
    auto entity = Entity();

    auto& motion = registry.motions.emplace(entity);
    motion.position = vec2(column * TILE_SIZE, row * TILE_SIZE);
    vec2 dimensions = { TILE_SIZE, TILE_SIZE };

    motion.center = {0.f,0.f};
    motion.collision_neg = {0.f,0.f};
    motion.collision_pos = dimensions;

    registry.sprites.insert(
            entity,
            {
                dimensions,
                TEXTURE_ASSET_ID::TILE_EXAMPLE
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
};
INTERNAL CurrentLevelData currentLevelData;

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
                    CreateBasicLevelTile(roomXIndex * r.width + j, roomYIndex * r.height + i);
                }break;
                case '1':{
                    currentLevelData.playerStart = { roomXIndex*r.width*TILE_SIZE + j*TILE_SIZE + (TILE_SIZE/2.f) ,
                                                     roomYIndex*r.height*TILE_SIZE + i*TILE_SIZE + (TILE_SIZE/2.f) };
                }break;
                default:{}break;
            }
        }
    }
}

#define NUMFLOORS 4
#define NUMROOMSWIDE 4

INTERNAL void GenerateNewLevel(u32 seed)
{
    srand(seed);

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

    // START ROOM
    u32 start = rand() % NUMROOMSWIDE;
    while(!roomDataArray[0][start].data.empty())
    {
        start = rand() % NUMROOMSWIDE;
    }
    u32 startRoomIndex = rand() % chapterOneRooms["start"].size();
    roomDataArray[0][start] = chapterOneRooms["start"][startRoomIndex];

    // END ROOM
    u32 end = rand() % NUMROOMSWIDE;
    while(!roomDataArray[NUMFLOORS-1][end].data.empty())
    {
        end = rand() % NUMROOMSWIDE;
    }
    u32 endRoomIndex = rand() % chapterOneRooms["end"].size();
    roomDataArray[NUMFLOORS-1][end] = chapterOneRooms["end"][endRoomIndex];

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

    for(int i = 0; i < NUMFLOORS; ++i)
    {
        for(int j = 0; j < NUMROOMSWIDE; ++j)
        {
            ParseRoomData(roomDataArray[i][j], j, i);
        }
    }

    for(int i = -1; i < 45; ++i)
    {
        CreateBasicLevelTile(i, -1);
        CreateBasicLevelTile(i, 36);
    }
    for(int i = -1; i < 37; ++i)
    {
        CreateBasicLevelTile(-1, i);
        CreateBasicLevelTile(44, i);
    }

    free(tileDataArray);
}
