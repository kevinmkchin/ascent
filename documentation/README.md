Provide a README.md providing entry points to each of the implemented features and
explain them where necessary.

Two procedurally generated levels:
	we have a system that takes a set of rooms and combines them randomly into a playable level.
	this is mostly in world_system.cpp and levels.cpp
Sprite sheet animation: 
	render system, render system init, and sprite system (a bit is in other parts like physics, ui, world init, levels.cpp...)
	the spritesheet itself is in data textures
Game logic response:
	mostly in AI system, and player system (reacting to button presses etc.)]
	the flying enemies (which use A*) are temporarily disabled (I'll be finishing them tomorrow likely
	I would've finished them sooner but I had covid followed by a pretty severe stomach flu which slowed me down immensely)
New integrated assets:
	assuming this is referring to art etc., you can see it in the data section specifically textures
Tutorial:
	its in the ui system primarily, best experienced in game
Sprite batching
	(?)
Parallax scrolling background:
	render system and world system
Projectile:
	visibile in component, collisions, texture