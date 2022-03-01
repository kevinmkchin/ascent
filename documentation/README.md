Provide a README.md providing entry points to each of the implemented features and
explain them where necessary.


# ascent

### CPSC 427 Team 9
- Sophie Berger 47805361
- Kevin Chin 88047956
- Chris Fung 38087904
- Caleb Kellett 44163533
- Sara Mohamed 95590519
- Suzuran Takikawa 79766572


Provide a README.md providing entry points to each of the implemented features and
explain them where necessary.


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
	- Render from and generate animations from spritesheets. Implementation found in render system, render system init, sprite system, and player system.
	- The spritesheet itself is in data/textures
	
- Game logic response:
	- mostly in AI system, and player system (reacting to button presses etc.)
	- the flying enemies (which use A*) are enabled or disabled through the setting: pathingBehavior.flyingType = true; in createEnemy() method in world_init.cpp
	
- New integrated assets:
	- New assets include background, sprites for level design, player, enemy, items, etc.
	- Found in data/textures
	
- Tutorial:
	- its in the ui system primarily, best experienced in game
	
- Sprite batching
	- Rendering identical sprites in batches, using radix sort
	- Implementation found in render system
	
- Parallax scrolling background:
	- Multi-layer background where each background moves at different speeds with player movement.
	- Implementation found in render system, world system, and background.vert
	
- Projectile:
	- visibile in component, collisions, texture
