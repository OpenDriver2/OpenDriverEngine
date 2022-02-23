#include "game/pch.h"

#include "shared/input.h"
#include "luabinding/lua_init.h"

#include <SDL_events.h>

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>


//---------------------------------------------------------------------------------------------------------------------------------

String					g_levname;
sol::state				g_luaState;

// stats counters
extern int g_drawnCells;
extern int g_drawnModels;
extern int g_drawnPolygons;
extern int g_cellsDrawDistance;

bool g_quit = false;
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
void UpdateStats(float deltaTime)
{
	CameraViewParams& view = CCamera::MainView;

	auto engineTable = g_luaState["engine"].get_or_create<sol::table>();

	auto statsTable = engineTable["Stats"].get_or_create<sol::table>();

	statsTable["systemFPS"] = UpdateFPSCounter(deltaTime);
	statsTable["cellsDrawDistance"] = g_cellsDrawDistance;
	statsTable["drawnCells"] = g_drawnCells;
	statsTable["drawnModels"] = g_drawnModels;
	statsTable["drawnPolygons"] = g_drawnPolygons;
}

//-------------------------------------------------------------
// SDL2 event handling
//-------------------------------------------------------------
void SDLPollEvent(sol::table& engineHostTable)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		bool anyWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

		CInput::UpdateEvents(event, engineHostTable, anyWindowFocused);

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
				bool down = (event.type == SDL_KEYDOWN);

				// lshift/right shift
				if (nKey == SDL_SCANCODE_RSHIFT)
					nKey = SDL_SCANCODE_LSHIFT;
				else if (nKey == SDL_SCANCODE_RCTRL)
					nKey = SDL_SCANCODE_LCTRL;
				else if (nKey == SDL_SCANCODE_RALT)
					nKey = SDL_SCANCODE_LALT;

				if (nKey == SDL_SCANCODE_PAGEUP && event.type == SDL_KEYDOWN)
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
void MainLoop()
{
	int64 oldTicks = Time::microTicks();
	IAudioSystem* audioSystem = IAudioSystem::Instance;

	// main loop
	do
	{
		// compute time
		const double ticks_to_ms = 1.0 / 1000000.0;
		int64 curTicks = Time::microTicks();

		float deltaTime = double(curTicks - oldTicks) * ticks_to_ms;

		oldTicks = curTicks;

		sol::table& engineHostTable = g_luaState["EngineHost"].get<sol::table>();

		SDLPollEvent(engineHostTable);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(g_window);
		ImGui::NewFrame();

		GR_BeginScene();

		GR_ClearDepth(1.0f);

		GR_SetCullMode(CULL_NONE);
		GR_SetBlendMode(BM_NONE);
		GR_SetDepthMode(1, 1);
		/*
		if (g_nightMode)
			GR_ClearColor(19 / 255.0f, 23 / 255.0f, 25 / 255.0f);
		else
			GR_ClearColor(128 / 255.0f, 158 / 255.0f, 182 / 255.0f);
			*/

		CManager_Players::Net_Update();

		if (engineHostTable.valid())
		{
			try {
				sol::function updateFunc = engineHostTable["Frame"];
				updateFunc(deltaTime);
			}
			catch (const sol::error& e)
			{
			}
		}

		// render main view
		if (CWorld::IsLevelLoaded())
		{
			CameraViewParams& view = CCamera::MainView;

			Matrix3x3 vectors = view.GetVectors();

			audioSystem->SetListener(view.position, CCamera::MainViewVelocity, vectors.rows[2], vectors.rows[1]);

			CSky::Draw(view);
			CWorld::RenderLevelView(view);
			CManager_Cars::Draw(view);

			CManager_Cars::UpdateTime(curTicks);
		}

		audioSystem->Update();

		CDebugOverlay::Draw();
		UpdateStats(deltaTime);

		try {
			sol::function updateFunc = engineHostTable["PostFrame"];
			updateFunc(deltaTime);
		}
		catch (const sol::error& e)
		{
		}

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

	LuaInit(g_luaState);
	UpdateStats(0.0f);

	if (!GR_Init("Driver", 1280, 720, 0))
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

	IAudioSystem::Create();

	CDebugOverlay::Init();

	CWorld::InitObjectMatrix();

	CWorld::InitHWTextures();
	CWorld::InitHWModels();

	CSky::Init();

	bool canRunLoop = true;

	// FIXME: force here?
	CManager_Players::Net_Init();

	// load lua script
	try {
		auto result = g_luaState.unsafe_script_file("scripts/_init.lua");
	}
	catch (const sol::error& e)
	{
		MsgError("_init error: %s\n", e.what());
		canRunLoop = false;
	}

	// loop and stuff
	if (canRunLoop)
	{
		MainLoop();
	}

	// free all
	CManager_Players::Net_Finalize();
	CWorld::UnloadLevel();
	IAudioSystem::Instance->Shutdown();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	CDebugOverlay::Destroy();
	GR_Shutdown();

	return 0;
}