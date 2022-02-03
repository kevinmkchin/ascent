// internal
#include "physics_system.hpp"
#include "world_init.hpp"

bool collides(Motion& motion1, Motion& motion2)
{
	vec2 max1 = motion1.position + (motion1.center + motion1.collision_pos) * abs(motion1.scale);
	vec2 min1 = motion1.position + (motion1.center - motion1.collision_neg) * abs(motion1.scale);

	vec2 max2 = motion2.position + (motion2.center + motion2.collision_pos) * abs(motion2.scale);
	vec2 min2 = motion2.position + (motion2.center - motion2.collision_neg) * abs(motion2.scale);

	if (min1.x <= max2.x && max1.x >= min2.x && min1.y <= max2.y && max1.y >= min2.y) {
	    // Calculate the x and y overlap between the two colliding entities
        float dx = min(max1.x, max2.x) - max(min1.x, min2.x);
        float dy = min(max1.y, max2.y) - max(min1.y, min2.y);

		// Reset collision overlaps of both entities
		motion1.collision_overlap = { 0, 0 };
		motion2.collision_overlap = { 0, 0 };

		Motion* motion_to_resolve = &motion1;
		// Pick the fastest moving entity to resolve/move back
		if (length(motion2.velocity) > length(motion1.velocity)) {
			motion_to_resolve = &motion2;
		}

		motion_to_resolve->collision_overlap = { max(0.f, dx), max(0.f, dy) };
        return true;
	}
	return false;
}

void PhysicsSystem::step(float deltaTime)
{
    float elapsed_ms = deltaTime * 1000.f;

	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
		motion.position += motion.velocity * step_seconds;
	}

	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);

				// Start resolving collision

				// Only one of the two entities should have a non-zero collision_overlap value
				Motion* motion_to_resolve = &motion_i;
				if (length(motion_j.collision_overlap) > 0) {
					motion_to_resolve = &motion_j;
				}

				// Here we find the shortest axis collision, this will be the axis that we resolve 

				// 0 for x, 1 for y
				int axis_to_resolve = 1;

				if (motion_to_resolve->collision_overlap.x < motion_to_resolve->collision_overlap.y) {
					axis_to_resolve = 0;
				}

				vec2 position_change = { 0, 0 };
				position_change[axis_to_resolve] = motion_to_resolve->collision_overlap[axis_to_resolve];

				// For now we only resolve the entity if it has velocity, might need to change if we want the non-moving player to be knocked back when hit by an attack or something
				if (motion_to_resolve->velocity[axis_to_resolve] > 0) {
					motion_to_resolve->position -= position_change;
				}
				else if (motion_to_resolve->velocity[axis_to_resolve] < 0) {
					motion_to_resolve->position += position_change;
				}
			}
		}
	}

	// debugging of bounding boxes
	if (debugging.in_debug_mode)
	{
		uint size_before_adding_new = (uint)motion_container.components.size();
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			Motion& motion_i = motion_container.components[i];
			Entity entity_i = motion_container.entities[i];
		}
	}
}