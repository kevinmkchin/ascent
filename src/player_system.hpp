#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "input.hpp"

class UISystem;
class WorldSystem;

class PlayerSystem
{
public:
	PlayerSystem();

	void Init(WorldSystem* world_sys_arg, UISystem* ui_sys_arg);

	void PausedStep(float deltaTime);

	void PrePhysicsStep(float deltaTime);

	void Step(float deltaTime);

	u8 lastAttackDirection = 0; // 0 left, 1 right, 2 up, 3 down

	float* GlobalPauseForSeconds = nullptr;

	bool bLeveledUpLastFrame = false;

private:
	void CheckIfLevelUp();

	Entity playerEntity;
	Player* playerComponentPtr;

	Entity playerMeleeAttackEntity;
	vec2 playerMeleeAttackPositionOffsetFromPlayer;
	float playerMeleeAttackCooldownTimer = -1.f;
	float playerMeleeAttackLengthTimer = -1.f;
	void PlayerAttackPrePhysicsStep(float deltaTime);
	void PlayerAttackStep();

	WorldSystem* world;
	UISystem* ui;
};