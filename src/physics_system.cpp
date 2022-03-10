// internal
#include "physics_system.hpp"
#include "world_init.hpp"

CollisionInfo CheckCollision(CollisionComponent& collider1, CollisionComponent& collider2)
{
	vec2 max1;
    max1.x = collider1.collider_position.x + ((float) collider1.collision_pos.x);
    max1.y = collider1.collider_position.y + ((float) collider1.collision_pos.y);
	vec2 min1;
    min1.x = collider1.collider_position.x + ((float) -collider1.collision_neg.x);
    min1.y = collider1.collider_position.y + ((float) -collider1.collision_neg.y);

    vec2 max2;
    max2.x = collider2.collider_position.x + ((float) collider2.collision_pos.x);
    max2.y = collider2.collider_position.y + ((float) collider2.collision_pos.y);
    vec2 min2;
    min2.x = collider2.collider_position.x + ((float) -collider2.collision_neg.x);
    min2.y = collider2.collider_position.y + ((float) -collider2.collision_neg.y);

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
        if(std::abs(motion.velocity.x) > std::abs(motion.terminalVelocity.x))
        {
            motion.velocity.x -= (motion.velocity.x/abs(motion.velocity.x)) * max(abs(motion.acceleration.x), 800.f) * deltaTime;
        }
        else
        {
            motion.velocity.x += motion.acceleration.x * deltaTime;
        }
        if(std::abs(motion.velocity.y) > std::abs(motion.terminalVelocity.y))
        {
            motion.velocity.y -= (motion.velocity.y/abs(motion.velocity.y)) * max(abs(motion.acceleration.y), 800.f) * deltaTime;
        }
        else
        {
            motion.velocity.y += motion.acceleration.y * deltaTime;
        }
        Entity e = motion_registry.entities[i];
        registry.transforms.get(e).position += motion.velocity * deltaTime;
        if(registry.colliders.has(e))
        {
            registry.colliders.get(e).collider_position = registry.transforms.get(e).position;
        }

        Entity& entity = motion_registry.entities[i];
        if (!registry.players.has(entity)) {
            if (motion.velocity.x > 0.f) {
                motion.facingRight = true;
            }
            else if (motion.velocity.x < 0.f) {
                motion.facingRight = false;
            }
        }
    }
}

struct ColEventWrapper {
    Entity e;
    CollisionEvent event;
};

bool ColEventWrapperSorter(ColEventWrapper const& lhs, ColEventWrapper const& rhs) 
{
    // Sort collision events by how much larger their y overlap is than x overlap
    // This makes it so we iterate over collisions that resolve vertically before looking at horizontal collisions
    return (abs(lhs.event.collision_overlap.y) - abs(lhs.event.collision_overlap.x)) 
            < (abs(rhs.event.collision_overlap.y) - abs(rhs.event.collision_overlap.x));
}

INTERNAL void CheckAllCollisions()
{
    // check all necessary entities
    std::vector<Entity> entitiesToCheck;
    entitiesToCheck.push_back(registry.players.entities[0]);

    std::vector<Entity> items = registry.items.entities;
    entitiesToCheck.insert(entitiesToCheck.end(), items.begin(), items.end());

    std::vector<Entity> enemies = registry.enemy.entities;
    entitiesToCheck.insert(entitiesToCheck.end(), enemies.begin(), enemies.end());

    std::vector<ColEventWrapper> colEventSortingVector;

    for(auto entity : entitiesToCheck)
    {
        if (registry.colliders.has(entity)) {
            CollisionComponent entityCollider = registry.colliders.get(entity);

            for (int i = 0; i < registry.colliders.size(); ++i)
            {
                auto e = registry.colliders.entities[i];
                if (e == entity) { continue; }
                CollisionComponent otherCollider = registry.colliders.components[i];

                if (length(entityCollider.collider_position - otherCollider.collider_position) > 64.f)
                {
                    continue; // if distance b/w is big then don't check
                }

                CollisionInfo colInfo = CheckCollision(entityCollider, otherCollider);
                if (colInfo.collides)
                {
                    CollisionEvent colEventAgainstOther(e);
                    CollisionEvent colEventAgainstEntity(entity);

                    colEventAgainstOther.collision_overlap = colInfo.collision_overlap;
                    colEventAgainstEntity.collision_overlap = -colInfo.collision_overlap;

                    colEventSortingVector.push_back({ entity, colEventAgainstOther });
                    colEventSortingVector.push_back({ e, colEventAgainstEntity });
                }
            }
        }
    }

    std::sort(colEventSortingVector.begin(), colEventSortingVector.end(), &ColEventWrapperSorter);

    for(const auto& sortedColEvent : colEventSortingVector)
    {
        registry.collisionEvents.insert(sortedColEvent.e, sortedColEvent.event, false);
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
