#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "input.hpp"

class PlayerSystem
{
public:
	PlayerSystem();

	void Step(float deltaTime);

private:
	u32 experiencePointsGained;
	u32 goldGained;

	Entity playerEntity;
};