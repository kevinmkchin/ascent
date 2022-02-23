#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "input.hpp"

class SpriteSystem
{
public:
	SpriteSystem();

	void Step(float deltaTime);
};