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