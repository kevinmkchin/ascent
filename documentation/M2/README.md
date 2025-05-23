# ascent

### CPSC 427 Team 9
- Sophie Berger 47805361
- Kevin Chin 88047956
- Chris Fung 38087904
- Caleb Kellett 44163533
- Sara Mohamed 95590519
- Suzuran Takikawa 79766572

### Features implemented in Milestone 2:
- Two procedurally generated levels:
	- We have a system that takes a set of rooms and combines them randomly into a playable level. Implementation found in world_system.cpp and levels.cpp.
	
- Sprite sheet animation: 
	- Render from and generate animations from spritesheets.
	- Implementation
		- components.hpp (Lines 174 - 211), render_system.cpp (Lines 285 - 325), player_system.cpp (Lines 292-361), sprite_system.cpp
	- The spritesheet itself is in data/textures
	
- Game logic response:
	- mostly in AI system, and player system (reacting to button presses etc.)
	- the flying enemies (which use A*) are enabled or disabled through the setting: pathingBehavior.flyingType = true; in createEnemy() method in world_init.cpp
	
- New integrated assets:
	- New assets include background, sprites for level design, player, enemy, items, etc.
	- Found in data/textures
	
- Tutorial:
	ui_system.cpp line 225
	
- Sprite batching
	- Rendering identical sprites in batches, using radix sort
	- Implementation found in render_system.cpp BatchDrawAllSprites line 160
	
- Parallax scrolling background:
	- Multi-layer background where each background moves at different speeds with player movement.
	- Implementation
		- render_system.cpp (Lines 141-158), background.vert
	
- Projectile:
	- visibile in component, collisions, texture
