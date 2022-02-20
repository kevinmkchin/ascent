// internal
#include "physics_system.hpp"
#include "world_init.hpp"

CollisionInfo CheckCollision(TransformComponent& transform1, CollisionComponent& collider1,
                             TransformComponent& transform2, CollisionComponent& collider2)
{
	vec2 max1 = transform1.position + (collider1.collision_pos) * abs(transform1.scale);
	vec2 min1 = transform1.position + (-collider1.collision_neg) * abs(transform1.scale);

	vec2 max2 = transform2.position + (collider2.collision_pos) * abs(transform2.scale);
	vec2 min2 = transform2.position + (-collider2.collision_neg) * abs(transform2.scale);

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

/** Move all entities that have a motion component */
INTERNAL void MoveEntities(float deltaTime)
{
    auto& motion_registry = registry.motions;
    for(u32 i = 0; i< motion_registry.size(); i++)
    {
        MotionComponent& motion = motion_registry.components[i];
        motion.velocity += motion.acceleration * deltaTime;
        motion.velocity.x = motion.velocity.x >= 0.f ? min(abs(motion.velocity.x), motion.terminalVelocity.x)
                : -min(abs(motion.velocity.x), motion.terminalVelocity.x);
        motion.velocity.y = motion.velocity.y >= 0.f ? min(abs(motion.velocity.y), motion.terminalVelocity.y)
                : -min(abs(motion.velocity.y), motion.terminalVelocity.y);
        registry.transforms.get(motion_registry.entities[i]).position += motion.velocity * deltaTime;
    }
}

INTERNAL void CheckAllCollisions()
{
//    // Check for collisions between all moving entities
//    ComponentContainer<MotionComponent> &motion_container = registry.motions;
//    for(uint i = 0; i<motion_container.components.size(); i++)
//    {
//        Motion& motion_i = motion_container.components[i];
//        Entity entity_i = motion_container.entities[i];
//
//        // note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
//        for(uint j = i+1; j<motion_container.components.size(); j++)
//        {
//            Motion& motion_j = motion_container.components[j];
//            Entity entity_j = motion_container.entities[j];
//
//            CollisionInfo colInfo = CheckCollision(motion_i, motion_j);
//            if (colInfo.collides)
//            {
//                // Create a collisions event
//                CollisionEvent colEventAgainstJ(entity_j);
//                CollisionEvent colEventAgainstI(entity_i);
//                // Note(Kevin): colInfo.collision_overlap is relative to entity_i, therefore
//                //              it needs to be inverted for colEventAgainstI
//                colEventAgainstJ.collision_overlap = colInfo.collision_overlap;
//                colEventAgainstI.collision_overlap = -colInfo.collision_overlap;
//
//                // We are abusing the ECS system a bit in that we potentially insert multiple collisions for the same entity
//                registry.collisionEvents.insert(entity_i, colEventAgainstJ, false);
//                registry.collisionEvents.insert(entity_j, colEventAgainstI, false);
//            }
//        }
//    }

    // check all necessary entities
    std::vector<Entity> entitiesToCheck;
    entitiesToCheck.push_back(registry.players.entities[0]);

    std::vector<Entity> items = registry.items.entities;
    entitiesToCheck.insert(entitiesToCheck.end(), items.begin(), items.end());

    std::vector<Entity> enemies = registry.enemy.entities;
    entitiesToCheck.insert(entitiesToCheck.end(), enemies.begin(), enemies.end());

    for(auto entity : entitiesToCheck)
    {
        TransformComponent entityTransform = registry.transforms.get(entity);
        CollisionComponent entityCollider = registry.colliders.get(entity);

        for(auto collidable : registry.colliders.entities)
        {
            if(collidable == entity) { continue; }

            TransformComponent otherTransform = registry.transforms.get(collidable);
            CollisionComponent otherCollider = registry.colliders.get(collidable);

            CollisionInfo colInfo = CheckCollision(entityTransform, entityCollider, otherTransform, otherCollider);
            if (colInfo.collides)
            {
                // Create a collisions event
                CollisionEvent colEventAgainstOther(collidable);
                CollisionEvent colEventAgainstEntity(entity);
                // Note(Kevin): colInfo.collision_overlap is relative to player, therefore
                //              it needs to be inverted for colEventAgainstEntity
                colEventAgainstOther.collision_overlap = colInfo.collision_overlap;
                colEventAgainstEntity.collision_overlap = -colInfo.collision_overlap;

                // We are abusing the ECS system a bit in that we potentially insert multiple collisions for the same entity
                registry.collisionEvents.insert(entity, colEventAgainstOther, false);
                registry.collisionEvents.insert(collidable, colEventAgainstEntity, false);
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
