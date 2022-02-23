#include "sprite_system.hpp"

SpriteSystem::SpriteSystem()
{

}

void SpriteSystem::Step(float deltaTime) {

    auto& sprite_registry = registry.sprites;
    for (u32 i = 0; i < sprite_registry.size(); i++)
    {
        SpriteComponent& sprite = sprite_registry.components[i];

        if (sprite.animations.empty()) {
            continue;
        }

        Animation& current_anim = sprite.animations[sprite.selected_animation];
        sprite.elapsed_time += deltaTime * 1000.f;

        if (sprite.elapsed_time >= (current_anim.animation_duration / current_anim.num_frames)) {
            sprite.elapsed_time = 0.f;
            sprite.current_frame++;
            if (sprite.current_frame >= current_anim.num_frames) {
                sprite.current_frame = 0;
            }
        }
    }
}