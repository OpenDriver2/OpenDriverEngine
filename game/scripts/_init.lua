-- TODO:
--		Implement dynamic script loader based on Updates.lua
--		Implement engine host sandboxing
--		Implement quick Lua script reloading
--		Implement menu and FPS meters hide (also need engine code)

MsgInfo("OpenDriverEngine Lua host initialization")

-- main OpenDriverEngine file
dofile "scripts/documentation.lua"
dofile "scripts/common.lua"
dofile "scripts/city.lua"
dofile "scripts/updates.lua"
dofile "scripts/camera.lua"
dofile "scripts/sound_bank.lua"
dofile "scripts/music.lua"
dofile "scripts/free_camera.lua"
dofile "scripts/main.lua"
