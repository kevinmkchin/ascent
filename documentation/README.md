# ascent

### CPSC 427 Team 9
- Sophie Berger 47805361
- Kevin Chin 88047956
- Chris Fung 38087904
- Caleb Kellett 44163533
- Sara Mohamed 95590519
- Suzuran Takikawa 79766572

### Milestone 1 Implemented Features
- Sprite renderer & SpriteComponent
  - render_system.cpp drawSprite line 6 finalDrawToScreen line 128, components.hpp line 111
- 2D AABB Collision detection & response  
  - physics_system.cpp CheckCollision line 5, world_system.cpp handle_collisions line 242
- Basic HealthComponent 
  - components.hpp line 152, world_system.cpp handle_collisions line 227
- Basic enemy 
  - world_init.cpp createEnemy line 100
- Player playerformer controller 
  - world_system.cpp 
- 2D Camera that follows player 
  - render_system.cpp drawSprite line 18
- Keyboard + gamepad input manager 
  - input.h, input.cpp
- Basic temporary level 
  - levels.cpp CreateBasicLevelTile line 6, world_system.cpp restart_game line 182

![](https://i.imgur.com/FbSTS7T.png)
