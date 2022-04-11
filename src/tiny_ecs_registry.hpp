#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<TransformComponent> transforms;
	ComponentContainer<MotionComponent> motions;
	ComponentContainer<CollisionComponent> colliders;
	ComponentContainer<CollisionEvent> collisionEvents;
	ComponentContainer<Player> players;
	ComponentContainer<SpriteComponent> sprites;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<HealthBar> healthBar;
	ComponentContainer<Enemy> enemy;
	ComponentContainer<EnemyProjectile> enemyProjectiles;
	ComponentContainer<ActiveMutationsComponent> mutations;
    ComponentContainer<Weapon> weapons;
    ComponentContainer<HolderComponent> holders;
    ComponentContainer<Item> items;
	ComponentContainer<ShopItem> shopItems;
	ComponentContainer<ActiveShopItem> activeShopItems;
	ComponentContainer<PathingBehavior> pathingBehaviors;
	ComponentContainer<PatrollingBehavior> patrollingBehaviors;
	ComponentContainer<FlyingBehavior> flyingBehaviors;
	ComponentContainer<WalkingBehavior> walkingBehaviors;
	ComponentContainer<RangedBehavior> rangedBehaviors;
	ComponentContainer<MeleeBehavior> meleeBehaviors;
	ComponentContainer<VisionComponent> visionComponents;
	ComponentContainer<DeathTimer> deathTimers;
    ComponentContainer<PlayerProjectile> playerProjectiles;
    ComponentContainer<ActivePlayerProjectile> activePlayerProjectiles;
	ComponentContainer<Exp> exp;
	ComponentContainer<Coin> coins;
	ComponentContainer<GoldBar> goldBar;
	ComponentContainer<ProximityTextComponent> proximityTexts;
	ComponentContainer<LightSource> lightSources;
	ComponentContainer<HealthPotion> healthPotion;
	ComponentContainer<EnemyMeleeAttack> enemyMeleeAttacks;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&transforms);
		registry_list.push_back(&motions);
		registry_list.push_back(&colliders);
		registry_list.push_back(&collisionEvents);
		registry_list.push_back(&players);
		registry_list.push_back(&sprites);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&healthBar);
		registry_list.push_back(&enemy);
		registry_list.push_back(&enemyProjectiles);
        registry_list.push_back(&mutations);
        registry_list.push_back(&weapons);
        registry_list.push_back(&holders);
        registry_list.push_back(&items);
		registry_list.push_back(&shopItems);
		registry_list.push_back(&activeShopItems);
		registry_list.push_back(&pathingBehaviors);
		registry_list.push_back(&patrollingBehaviors);
		registry_list.push_back(&flyingBehaviors);
		registry_list.push_back(&walkingBehaviors);
		registry_list.push_back(&rangedBehaviors);
		registry_list.push_back(&meleeBehaviors);
		registry_list.push_back(&visionComponents);
		registry_list.push_back(&deathTimers);
        registry_list.push_back(&playerProjectiles);
        registry_list.push_back(&activePlayerProjectiles);
		registry_list.push_back(&exp);
		registry_list.push_back(&coins);
		registry_list.push_back(&goldBar);
		registry_list.push_back(&proximityTexts);
		registry_list.push_back(&lightSources);
		registry_list.push_back(&healthPotion);
		registry_list.push_back(&enemyMeleeAttacks);
		
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;