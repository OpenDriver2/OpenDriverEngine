
#include <string.h>
#include <SDL_events.h>

#include "core/cmdlib.h"
#include "core/VirtualStream.h"

#include <nstd/Array.hpp>
#include <nstd/File.hpp>
#include <nstd/Directory.hpp>
#include <nstd/String.hpp>
#include <nstd/Time.hpp>

#include "routines/level.h"
#include "routines/textures.h"
#include "routines/regions.h"

#include "core/ignore_vc_new.h"
#include <sol/sol.hpp>

#include "game/shared/manager_cars.h"

#include "renderer/debug_overlay.h"
#include "renderer/gl_renderer.h"

#include "shared/world.h"
#include "shared/camera.h"

#include "game/render/render_level.h"
#include "game/render/render_model.h"
#include "game/render/render_sky.h"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"


//---------------------------------------------------------------------------------------------------------------------------------

String					g_levname;
sol::state				g_luaState;

// stats counters
extern int g_drawnCells;
extern int g_drawnModels;
extern int g_drawnPolygons;

bool g_quit = false;

bool g_nightMode = false;
bool g_displayCollisionBoxes = false;
bool g_displayHeightMap = false;
bool g_displayAllCellLevels = true;
bool g_noLod = false;

int g_cellsDrawDistance = 441;

int g_currentModel = 0;

bool g_holdLeft = false;
bool g_holdRight = false;
bool g_holdShift = false;

//-------------------------------------------------------------
// Updates frames per second counter and returns a number
//-------------------------------------------------------------
int UpdateFPSCounter(float deltaTime)
{
	// Engine frames status
	static float accumTime = 0.1f;
	static int framesPerSecond = 0;
	static int numFrames = 0;

	if (accumTime > 0.1f)
	{
		framesPerSecond = (int)((float)numFrames / accumTime + 0.5f);
		numFrames = 0;
		accumTime = 0;
	}

	accumTime += deltaTime;
	numFrames++;

	return framesPerSecond;
}

//-------------------------------------------------------------
// Displays Main menu bar, stats and child windows
//-------------------------------------------------------------
void DisplayUI(float deltaTime)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Change level..."))
			{
				MsgWarning("TODO: change level file!");
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Level"))
		{
			if (ImGui::MenuItem("Spool all Area Data"))
				CWorld::SpoolAllAreaDatas();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Night mode", nullptr, g_nightMode))
				g_nightMode ^= 1;

			if (ImGui::MenuItem("Disable LODs", nullptr, g_noLod))
				g_noLod ^= 1;

			ImGui::Separator();

			if (ImGui::MenuItem("Display collision boxes", nullptr, g_displayCollisionBoxes))
				g_displayCollisionBoxes ^= 1;

			if (ImGui::MenuItem("Display heightmap", nullptr, g_displayHeightMap))
				g_displayHeightMap ^= 1;

			if (ImGui::MenuItem("Display hidden objects", nullptr, g_displayAllCellLevels))
				g_displayAllCellLevels ^= 1;

			ImGui::Separator();

			if (ImGui::MenuItem("Reset camera", nullptr, g_noLod))
			{
				g_cameraPosition = 0;
				g_cameraAngles = Vector3D(25.0f, 45.0f, 0);

				//g_cameraPosition = FromFixedVector({ 230347, 372, 704038 });
				//g_cameraAngles = FromFixedVector({ 0, 3840 - 1024, 0 }) * 360.0f;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin("HelpFrame", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiInputTextFlags_NoHorizontalScroll |
		ImGuiWindowFlags_NoSavedSettings | ImGuiColorEditFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		ImGui::SetWindowPos(ImVec2(0, 24));

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "FPS: %d", UpdateFPSCounter(deltaTime));

		ImGui::SetWindowSize(ImVec2(400, 120));

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.25f, 1.0f), "Position: X: %d Y: %d Z: %d",
			int(g_cameraPosition.x * ONE_F), int(g_cameraPosition.y * ONE_F), int(g_cameraPosition.z * ONE_F));

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Draw distance: %d", g_cellsDrawDistance);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Drawn cells: %d", g_drawnCells);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Drawn models: %d", g_drawnModels);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Drawn polygons: %d", g_drawnPolygons);

		ImGui::End();
	}
}

//-------------------------------------------------------------
// SDL2 event handling
//-------------------------------------------------------------
void SDLPollEvent()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		bool anyWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

		switch (event.type)
		{
		case SDL_QUIT:
		{
			g_quit = 1;
			break;
		}
		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				GR_UpdateWindowSize(event.window.data1, event.window.data2);

				break;
			case SDL_WINDOWEVENT_CLOSE:
				g_quit = 1;
				break;
			}
			break;
		}
		case SDL_MOUSEMOTION:
		{
			if (anyWindowFocused)
				break;

			if (g_holdLeft)
			{
				g_cameraAngles.x += event.motion.yrel * 0.25f;
				g_cameraAngles.y -= event.motion.xrel * 0.25f;
			}

			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			bool down = (event.type == SDL_MOUSEBUTTONDOWN);

			if (anyWindowFocused)
				down = false;

			if (event.button.button == 1)
				g_holdLeft = down;
			else if (event.button.button == 3)
				g_holdRight = down;
			break;
		}
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			int nKey = event.key.keysym.scancode;

			// lshift/right shift
			if (nKey == SDL_SCANCODE_RSHIFT)
				nKey = SDL_SCANCODE_LSHIFT;
			else if (nKey == SDL_SCANCODE_RCTRL)
				nKey = SDL_SCANCODE_LCTRL;
			else if (nKey == SDL_SCANCODE_RALT)
				nKey = SDL_SCANCODE_LALT;

			if (nKey == SDL_SCANCODE_LSHIFT || nKey == SDL_SCANCODE_RSHIFT)
				g_holdShift = (event.type == SDL_KEYDOWN);
			else if (nKey == SDL_SCANCODE_LEFT)
			{
				g_cameraMoveDir.x = (event.type == SDL_KEYDOWN) ? -1.0f : 0.0f;
			}
			else if (nKey == SDL_SCANCODE_RIGHT)
			{
				g_cameraMoveDir.x = (event.type == SDL_KEYDOWN) ? 1.0f : 0.0f;
			}
			else if (nKey == SDL_SCANCODE_UP)
			{
				g_cameraMoveDir.z = (event.type == SDL_KEYDOWN) ? 1.0f : 0.0f;
			}
			else if (nKey == SDL_SCANCODE_DOWN)
			{
				g_cameraMoveDir.z = (event.type == SDL_KEYDOWN) ? -1.0f : 0.0f;
			}
			else if (nKey == SDL_SCANCODE_PAGEUP && event.type == SDL_KEYDOWN)
			{
				g_cellsDrawDistance += 441;
			}
			else if (nKey == SDL_SCANCODE_PAGEDOWN && event.type == SDL_KEYDOWN)
			{
				g_cellsDrawDistance -= 441;
				if (g_cellsDrawDistance < 441)
					g_cellsDrawDistance = 441;
			}

			break;
		}
		}
	}
}

extern SDL_Window* g_window;

//-------------------------------------------------------------
// Loop
//-------------------------------------------------------------
void ViewerMainLoop()
{
	int64 oldTicks = Time::microTicks();

	// main loop
	do
	{
		// compute time
		const double ticks_to_ms = 1.0 / 1000000.0;
		int64 curTicks = Time::microTicks();

		float deltaTime = double(curTicks - oldTicks) * ticks_to_ms;

		oldTicks = curTicks;

		SDLPollEvent();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(g_window);
		ImGui::NewFrame();

		GR_BeginScene();

		GR_ClearDepth(1.0f);
		GR_SetCullMode(CULL_NONE);
		GR_SetBlendMode(BM_NONE);
		GR_SetDepth(1);

		if (g_nightMode)
			GR_ClearColor(19 / 255.0f, 23 / 255.0f, 25 / 255.0f);
		else
			GR_ClearColor(128 / 255.0f, 158 / 255.0f, 182 / 255.0f);

		// Render stuff
		float cameraSpeedModifier = g_holdShift ? 4.0f : 1.0f;

		UpdateCameraMovement(deltaTime, cameraSpeedModifier);

		CSky::Draw();

		CWorld::RenderLevelView();

		CDebugOverlay::Draw();

		// Do ImGUI interface
		DisplayUI(deltaTime);

		// draw stuff
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		GR_EndScene();
		GR_SwapWindow();

	} while (!g_quit);
}

//-------------------------------------------------------------
// Main level viewer
//-------------------------------------------------------------
int main(int argc, char* argv[])
{
#ifdef _WIN32
	Install_ConsoleSpewFunction();
#endif

	Msg("---------------\nOpenDriverEngine startup\n---------------\n\n");

	// TODO: lua_init.cpp
	g_luaState.open_libraries(sol::lib::base);

	CManager_Cars::Lua_Init(g_luaState);
	CDebugOverlay::Lua_Init(g_luaState);
	CWorld::Lua_Init(g_luaState);
	CSky::Lua_Init(g_luaState);

	if (!GR_Init("Open Driver Engine", 1280, 720, 0))
	{
		MsgError("Failed to init graphics!\n");
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	//ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(g_window, nullptr);
	ImGui_ImplOpenGL3_Init();

	CDebugOverlay::Init();

	CWorld::InitHWTextures();
	CWorld::InitHWModels();

	// load lua script
	try {
		auto result = g_luaState.safe_script_file("scripts/_init.lua");
	}
	catch (const sol::error& e)
	{
		MsgError("_init error: %s\n", e.what());

		GR_Shutdown();
		return -1;
	}

	/*
	// Load level file
	if (!CWorld::LoadLevelFile(g_levname))
	{
		GR_Shutdown();
		return -1;
	}

	//CSky::Init("DRIVER2/DATA/SKY2.RAW", 1);
	*/

	// loop and stuff
	ViewerMainLoop();

	// free all
	CWorld::FreeLevelData();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	CDebugOverlay::Destroy();
	GR_Shutdown();

	return 0;
}