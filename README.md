# OpenDriverEngine - Open source implementation of Driver 1/2 engine

### Requirements specification (short version):

Code:
- Project takes REDRIVER2 as a basis for the work, however, the code for it should be written from scratch.
- Written in **C++** (no C++ STD - instead, **libnstd** is used)
- Dependency list should be *short*, dependecies must be *lightweight as possible except renderer*

Current task:
- Completely rewrite REDRIVER2 to **C++** and remove PSX dependencies

Engine capabilities requirements:
- Vector Math library - from **Equilibrium Engine**. Exception is a fixed point math that should replicate some of GTE functionality for compatibility
- Filesystem library - MAYBE from **Equilibrium Engine**.
- Scripting - **Lua 5.4** - for suporting fixed point math. Should be enough performance-wise
- Configuration files - **JSON** or directly **Lua**. Also Lua has plenty of decent JSON libraries
- Renderer - starting with **DriverLevelTool** OpenGL renderen moving to **DiligentEngine** middleware that supports *Vulkan, DirectX11/12, OpenGL* and shader cross-compilation.
- Sound engine - with use of **OpenAL**? allowing to load *WAV/OGG* files and stream if needed.
- World and object system - extensible system allowing to run not only *Driver 1 and Driver 2*, but make potentially any driving game compatible with OpenDriverEngine
- Custom **model format** and **exporter for Blender**
- Textures using **TGA** and **DDS** format.

Compatibility
- *Driver 2* content should be processed by converter to make games run instantly on *OpenDriverEngine*
- *Driver 1* content level files can be processed by converter. The rest is recreated with *Lua scripts*
- Untouched original *Driver 2* physics, but extended it with new features

Gameplay
- *Smooth gameplay* - original *30 FPS* or *50 FPS* car physics interpolated to **60 FPS or greater**
- Full game engine scripting in **Lua 5.4** taking missions, sound banks, frontend into account to make game extensible with new features
- Game logic of minigames and Undercover missions port to Lua
- *Driver 2 AI port to Lua* and ability to build new AI with Lua
