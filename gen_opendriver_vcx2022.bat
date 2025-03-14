set PREMAKE_FILE=%cd%\premake5.lua

cd eq2engine
utils\premake5 --file=%PREMAKE_FILE% vs2022
pause