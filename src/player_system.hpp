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

	void PausedStep(float deltaTime);

	void PrePhysicsStep(float deltaTime);

	void Step(float deltaTime);

	u8 lastAttackDirection = 0; // 0 left, 1 right, 2 up, 3 down

	float* GlobalPauseForSeconds = nullptr;

	bool bLeveledUpLastFrame = false;

private:
	void CheckIfLevelUp();

	Entity playerEntity;

	Entity playerMeleeAttackEntity;
	float playerMeleeAttackCooldownTimer = -1.f;
	void PlayerAttackPrePhysicsStep(float deltaTime);
	void PlayerAttackStep();
};