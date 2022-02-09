// internal
#include "physics_system.hpp"
#include "world_init.hpp"

CollisionInfo CheckCollision(Motion& motion1, Motion& motion2)
{
	vec2 max1 = motion1.position + (motion1.collision_pos) * abs(motion1.scale);
	vec2 min1 = motion1.position + (-motion1.collision_neg) * abs(motion1.scale);

	vec2 max2 = motion2.position + (motion2.collision_pos) * abs(motion2.scale);
	vec2 min2 = motion2.position + (-motion2.collision_neg) * abs(motion2.scale);

    CollisionInfo cinfo;

	if (min1.x < max2.x && max1.x > min2.x && min1.y < max2.y && max1.y > min2.y) {

        // Note(Kevin): doesn't work when one box is fully enveloped by the other box
	    // Calculate the x and y overlap between the two colliding entities
        float dx = min(max1.x, max2.x) - max(min1.x, min2.x);
        float dy = min(max1.y, max2.y) - max(min1.y, min2.y);

        if(max1.x - min2.x == dx)
        {
            dx = -dx;
        }
        if(max1.y - min2.y == dy)
        {
            dy = -dy;
        }

		cinfo.collision_overlap = { dx, dy };
        cinfo.collides = true;
        return cinfo;
	}

    cinfo.collides = false;
	return cinfo;
}

INTERNAL void MoveEntities(float deltaTime)
{
    auto& motion_registry = registry.motions;
    for(u32 i = 0; i< motion_registry.size(); i++)
    {
        Motion& motion = motion_registry.components[i];
        motion.velocity += motion.acceleration * deltaTime;
        motion.velocity.x = motion.velocity.x >= 0.f ? min(abs(motion.velocity.x), motion.terminalVelocity.x)
                : -min(abs(motion.velocity.x), motion.terminalVelocity.x);
        motion.velocity.y = motion.velocity.y >= 0.f ? min(abs(motion.velocity.y), motion.terminalVelocity.y)
                : -min(abs(motion.velocity.y), motion.terminalVelocity.y);
        motion.position += motion.velocity * deltaTime;
    }
}

INTERNAL void CheckAllCollisions()
{
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

            CollisionInfo colInfo = CheckCollision(motion_i, motion_j);
            if (colInfo.collides)
            {
                // Create a collisions event
                CollisionEvent colEventAgainstJ(entity_j);
                CollisionEvent colEventAgainstI(entity_i);
                // Note(Kevin): colInfo.collision_overlap is relative to entity_i, therefore
                //              it needs to be inverted for colEventAgainstI
                colEventAgainstJ.collision_overlap = colInfo.collision_overlap;
                colEventAgainstI.collision_overlap = -colInfo.collision_overlap;

                // We are abusing the ECS system a bit in that we potentially insert multiple collisions for the same entity
                registry.collisionEvents.insert(entity_i, colEventAgainstJ, false);
                registry.collisionEvents.insert(entity_j, colEventAgainstI, false);
            }
        }
    }
}

INTERNAL void DoDebugging()
{
    // debugging of bounding boxes
    if (debugging.in_debug_mode)
    {

    }
}

void PhysicsSystem::step(float deltaTime)
{
    MoveEntities(deltaTime);
    CheckAllCollisions();
    DoDebugging();
}
