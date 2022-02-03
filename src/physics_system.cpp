// internal
#include "physics_system.hpp"
#include "world_init.hpp"

struct CollisionInfo
{
    vec2 collision_overlap = {0.f, 0.f};
    bool collides = false;
};

CollisionInfo collides(Motion& motion1, Motion& motion2)
{
	vec2 max1 = motion1.position + (motion1.center + motion1.collision_pos) * abs(motion1.scale);
	vec2 min1 = motion1.position + (motion1.center - motion1.collision_neg) * abs(motion1.scale);

	vec2 max2 = motion2.position + (motion2.center + motion2.collision_pos) * abs(motion2.scale);
	vec2 min2 = motion2.position + (motion2.center - motion2.collision_neg) * abs(motion2.scale);

    CollisionInfo cinfo;

	if (min1.x < max2.x && max1.x > min2.x && min1.y < max2.y && max1.y > min2.y) {
	    // Calculate the x and y overlap between the two colliding entities
        float dx = min(max1.x, max2.x) - max(min1.x, min2.x);
        float dy = min(max1.y, max2.y) - max(min1.y, min2.y);

		cinfo.collision_overlap = { max(0.f, dx), max(0.f, dy) };
        cinfo.collides = true;
        return cinfo;
	}

    cinfo.collides = false;
	return cinfo;
}

void PhysicsSystem::step(float deltaTime)
{
    float elapsed_ms = deltaTime * 1000.f;

	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
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
            Entity entity_j = motion_container.entities[j];

            CollisionInfo colInfo = collides(motion_i, motion_j);
			if (colInfo.collides)
			{
				// Create a collisions event
                Collision colEventJ(entity_j);
                Collision colEventI(entity_i);
                colEventJ.collision_overlap = colInfo.collision_overlap;
                colEventI.collision_overlap = colInfo.collision_overlap;

				// We are abusing the ECS system a bit in that we potentially insert multiple collisions for the same entity
				registry.collisions.insert(entity_i, colEventJ, false);
				registry.collisions.insert(entity_j, colEventI, false);
			}
		}
	}

	// debugging of bounding boxes
	if (debugging.in_debug_mode)
	{

	}
}