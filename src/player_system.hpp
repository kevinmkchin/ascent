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

	void PrePhysicsStep(float deltaTime);

	void Step(float deltaTime);

private:
	void CheckIfLevelUp();

	Entity playerEntity;

	Entity playerMeleeAttackEntity;
	float playerMeleeAttackCooldownTimer = -1.f;
	void PlayerAttackPrePhysicsStep(float deltaTime);
	void PlayerAttackStep();
};