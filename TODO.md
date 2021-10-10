----------------------------------
OpenDriverEngine 
**TODO list**
----------------------------------

LEGEND:

**IN PROGRESS**		- work in progress
**DONE**				- implemented and completed

----------------------------------

- Driver 1/2 level loading and management
	- - **DONE** Load LEV files
	- - **DONE** Load textures
	- - **DONE** Load models
	- - **DONE** Load spooled regions
	- - **DONE** Load roads
	- - **DONE** Driver 2 heightmaps
	- - **IN PROGRESS** Driver 1 heightmaps
		BUG BUG: some road map data heights are not valid!

- Sounds
	- - **DONE** Sound system
	- - **DONE** WAV loading
	- - **DONE** OGG loading
	- - **DONE** Audible source management
	- - Advanced sound effects (reverb, echo etc)
	- - Effect zones

- Rendering
	- - **DONE** Driver 2 skies
	- - **IN PROGRESS** Driver 1 skies
	- - Loading textures from external files (overrides)
	- - Loading models from external files (overrides)
	- - Vertex or per-pixel lighting system
	- - sun light shadow mapping

- Special effects
	- - Level lighting management
	- - Smashable objects
	- - Smoke particles
	- - Debris particles
	- - Spark particles
	- - Rain particles
	- - Snow particles

- Driver 2 Game physics
	- - **DONE** Dynamics
	- - **DONE** Car wheels
	- - **DONE** World Collisions
	- - **DONE** Car vs Car collisions
	- - **DONE** Collision events
	- - **DONE** Lua Collision Events
	- - Other miscellaneous physics object support

- Driver 1 Game physics
	- - **DONE** Decompile and reimplement car cosmetics
	- - **DONE** Driver 2 car phyiscs tweaks for compatibility

- Event / freely placed objects
	- - **IN PROGRESS** cell objects adding to world
	- - non-cell objects adding to world
	- - Event objects management
	- - Lua-controlled surface generation and transformation
	- - Lua-controlled groupped cell object transformation
		- - - Chicago bridges
		- - - Ferries

- Pathfinding AI system
	- - Direct port of Driver 2 pathfinding AI
	- - per-pursued car (for each player) basis for pathfinding AI

- AI system interface
	- - Driver 2 Roads system
	- - Driver 1 Roads system
	- - Driver 2 pedestrian roads system
	- - Driver 1 pedestrian roads system
	- - Shared Road system interface

- City AI
	- - Civ car spawning
	- - Civ AI controller
	- - Cop AI controller

- Pedestrian system
	- - TODO

- Player manager implementation
	- - TODO

- Replay system implementation
	- - TODO

- Online system implementation
	- - TODO