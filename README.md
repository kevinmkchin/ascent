# ascent

### CPSC 427 Team 9
- Sophie Berger 47805361
- Kevin Chin 88047956
- Chris Fung 38087904
- Caleb Kellett 44163533
- Sara Mohamed 95590519
- Suzuran Takikawa 79766572

### Features implemented in Milestone 3:

- Parallax background in main menu: 
	- Visible in worldSystem.cpp -> SetCurrentMode()
	- Visible in RenderSystem.cpp -> DrawAllBackgrounds()

- Bouncing off of ground enemies:
	- Visible in WorldSystem.cpp -> handle_collisions()

- Updated flying enemy animations and sprite sheet:
	- Visible in world_init.cpp

- Coins and exp drop after enemy death + animation 
	- Visible in world_init.cpp (Lines 834-942)
	- Visible in worldSystem.cpp
	- Visible in components.hpp

- Pathing etc.
	- Visible in ai_system.cpp primarily, supported by components system and world_init

- multiple difficulties:
	- Visible everywhere enum DIFFICULTY (found in world_system.cpp) members are used, primarily world_system.cpp
	- button is in ui system and main