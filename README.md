# OD2Engine - Open source implementation of Driver 1/2 engine

Project takes REDRIVER2 as a basis for the work, however, mostly the code for it should be written from scratch.

Short requirements specification:
- Written in **C++** (no ugly or less of STL API, utilize only REALLY needed functionality from it. Or maybe use different STL)
- Dependency list should be *short*, dependecies must be *lightweight as possible*
- New *graphics and sound engine*
- Untouched original *Driver 2* physics, extending it with new features
- *Driver 2 and Driver 1 PC and PS1* levels support (unpacked resources)
- *Smooth gameplay* - original *30 FPS* car physics interpolated to **60 FPS and more**
- Full game engine scripting in **Lua 5.4** taking missions, sound banks, frontend into account to make game extensible with new features
- *Same AI* and ability to build new AI
- **High compatibility with Driver 2 and Driver 1**
