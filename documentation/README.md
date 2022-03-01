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
	we have a system that takes a set of rooms and combines them randomly into a playable level.
	this is mostly in world_system.cpp and levels.cpp
	
- Sprite sheet animation: 
	render system, render system init, and sprite system (a bit is in other parts like player system, physics, ui, world init, levels.cpp...)
	the spritesheet itself is in data textures
	
- Game logic response:
	mostly in AI system, and player system (reacting to button presses etc.)]
	the flying enemies (which use A*) are enabled or disabled through the setting: pathingBehavior.flyingType = true; in createEnemy() method in world_init.cpp

- New integrated assets:
	assuming this is referring to art etc., you can see it in the data section specifically textures
	
- Tutorial:
	ui_system.cpp line 225
	
- Sprite batching
	render_system.cpp BatchDrawAllSprites line 160
	
- Parallax scrolling background:
	render system and world system
	
- Projectile:
	visibile in component, collisions, texture
