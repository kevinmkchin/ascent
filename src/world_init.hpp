#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

Entity createBox(vec2 position);

Entity createPlayer(vec2 position);

Entity CreateBatEnemy(vec2 position);

Entity CreateKnightEnemy(vec2 position);

Entity createEnemyProjectile(vec2 position, vec2 velocity, Entity enemy);

Entity createSword(vec2 position);

Entity CreateShopItem(vec2 position, u8 shopItemIndex);