#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

Entity createBox(vec2 position);

Entity createPlayer(vec2 position);

Entity CreateBatEnemy(vec2 position);

Entity CreateKnightEnemy(vec2 position);

Entity CreateStationaryEnemy(vec2 position);

Entity CreateGoblinEnemy(vec2 position);

Entity CreateGoblinBomberEnemy(vec2 position);

Entity CreateMushroomEnemy(vec2 position);

Entity CreateSlimeEnemy(vec2 position);

Entity CreateWormEnemy(vec2 position);

Entity createEnemyLobbingProjectile(vec2 position, vec2 velocity, vec2 acceleration, Entity enemy);

Entity createEnemyProjectile(vec2 position, vec2 velocity, Entity enemy);

Entity createExp(vec2 position);

Entity createSword(vec2 position);

Entity createBow(vec2 position);

Entity createWalkingBomb(vec2 position);

Entity CreateShopItem(vec2 position, u8 shopItemIndex);

Entity createCoins(vec2 position);

Entity CreateProximityWorldText(vec2 pos, float triggerRadius, u32 size, const char* text);

Entity CreateHelpSign(vec2 pos, float triggerRadius, vec2 textOffsetFromPos, u32 textSize, const char* text);

Entity CreateShopKeeperNPC(vec2 position);

Entity createHealthPotion(vec2 position);