# OpenDriverEngine - Open source implementation of Driver 1/2 engine

### Requirements specification (short version):

Code:
- Project takes REDRIVER2 as a basis for the work, however, the code for it should be written from scratch.
- Written in **C++** (no C++ STD - instead, **libnstd** is used)
- Dependency list should be *short*, dependecies must be *lightweight as possible except renderer*

Compatibility
- *Driver 2* content should be processed by converter to make games run instantly on *OpenDriverEngine*
- *Driver 1* content level files can be processed by converter. The rest is recreated with *Lua scripts*
- Untouched original *Driver 2* physics, but extended it with new features

Gameplay
- *Smooth gameplay* - original *30 FPS* or *50 FPS* car physics interpolated to **60 FPS or greater**
- Full game engine scripting in **Lua 5.4** taking missions, sound banks, frontend into account to make game extensible with new features
- Game logic of minigames and Undercover missions port to Lua
- *Driver 2 AI port to Lua* and ability to build new AI with Lua

Please look at [https://github.com/OpenDriver2/OpenDriverEngine/blob/master/TODO.md](TODO.md) for more information