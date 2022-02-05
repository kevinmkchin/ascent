#include <fstream>
#include <iostream>
#include <json.hpp>

#include "common.hpp"
#include "tiny_ecs_registry.hpp"

#define TILE_SIZE 16

INTERNAL Entity CreateBasicLevelTile(u32 column, u32 row)
{
    auto entity = Entity();

    auto& motion = registry.motions.emplace(entity);
    motion.position = vec2(column * TILE_SIZE, row * TILE_SIZE);
    vec2 dimensions = { TILE_SIZE, TILE_SIZE };

    motion.center = {0.f,0.f};
    motion.collision_neg = {0.f,0.f};
    motion.collision_pos = dimensions;

    registry.eatables.emplace(entity);
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
        int width;
        int height;
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

    std::ifstream ifs = std::ifstream(level_path("filename.json"));
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

INTERNAL void GenerateNewLevel()
{
    printf("%s\n", chapterOneRooms.at("shop").at(0).name.c_str());
    printf("%s\n", chapterOneRooms.at("shop").at(0).data.c_str());
    printf("%s\n", chapterOneRooms.at("shop").at(1).name.c_str());
    printf("%s\n", chapterOneRooms.at("shop").at(1).data.c_str());
}
