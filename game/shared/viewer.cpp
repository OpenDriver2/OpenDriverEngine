#include <malloc.h>
#include <SDL_events.h>

#include <nstd/Array.hpp>
#include <nstd/Directory.hpp>
#include <nstd/String.hpp>
#include <nstd/Time.hpp>

#include "camera.h"
#include "convert.h"
#include "debug_overlay.h"
#include "driver_level.h"
#include "gl_renderer.h"
#include "renderlevel.h"
#include "rendermodel.h"

#include "core/cmdlib.h"
#include "core/VirtualStream.h"

#include "math/Volume.h"

#include "driver_routines/d2_types.h"
#include "driver_routines/level.h"
#include "driver_routines/models.h"
#include "driver_routines/regions_d1.h"
#include "driver_routines/regions_d2.h"
#include "driver_routines/textures.h"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"

bool g_quit = false;

bool g_nightMode = false;
bool g_displayCollisionBoxes = false;
bool g_displayHeightMap = false;
bool g_displayAllCellLevels = true;
bool g_noLod = false;

int g_cellsDrawDistance = 441;

int g_currentModel = 0;

int g_viewerMode = 0;

bool g_holdLeft = false;
bool g_holdRight = false;
bool g_holdShift = false;

//-----------------------------------------------------------------

TextureID g_hwTexturePages[128][32];
extern TextureID g_whiteTexture;

// Creates hardware texture
void InitHWTexturePage(CTexturePage* tpage)
{
	const TexBitmap_t& bitmap = tpage->GetBitmap();

	if (bitmap.data == nullptr)
		return;

	int imgSize = TEXPAGE_SIZE * 4;
	uint* color_data = (uint*)malloc(imgSize);

	memset(color_data, 0, imgSize);

	// Dump whole TPAGE indexes
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			ubyte clindex = bitmap.data[y * 128 + (x >> 1)];

			if (0 != (x & 1))
				clindex >>= 4;

			clindex &= 0xF;

			int ypos = (TEXPAGE_SIZE_Y - y - 1) * TEXPAGE_SIZE_Y;

			color_data[ypos + x] = clindex * 32;
		}
	}

	int numDetails = tpage->GetDetailCount();

	// FIXME: load indexes instead?

	for (int i = 0; i < numDetails; i++)
		tpage->ConvertIndexedTextureToRGBA(color_data, i, &bitmap.clut[i], false, false);

	int tpageId = tpage->GetId();
	
	g_hwTexturePages[tpageId][0] = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);

	// also load different palettes
	int numPalettes = 0;
	for (int pal = 0; pal < 16; pal++)
	{
		bool anyMatched = false;

		for (int j = 0; j < numDetails; j++)
		{
			TexDetailInfo_t* detail = tpage->GetTextureDetail(j);

			if (detail->extraCLUTs[pal])
			{
				tpage->ConvertIndexedTextureToRGBA(color_data, j, detail->extraCLUTs[pal], false, false);
				anyMatched = true;
			}
		}

		if (anyMatched)
		{
			g_hwTexturePages[tpageId][numPalettes + 1] = GR_CreateRGBATexture(TEXPAGE_SIZE_Y, TEXPAGE_SIZE_Y, (ubyte*)color_data);
			numPalettes++;
		}
	}
	
	
	// no longer need in RGBA data
	free(color_data);
}

void FreeHWTexturePage(CTexturePage* tpage)
{
	int tpageId = tpage->GetId();
	GR_DestroyTexture(g_hwTexturePages[tpageId][0]);

	for (int pal = 0; pal < 16; pal++)
	{
		if(g_hwTexturePages[tpageId][pal + 1] != g_whiteTexture)
			GR_DestroyTexture(g_hwTexturePages[tpageId][pal + 1]);
	}
}

// returns hardware texture
TextureID GetHWTexture(int tpage, int pal)
{
	if (tpage < 0 || tpage >= 128 ||
		pal < 0 || pal >= 32)
		return g_whiteTexture;

	return g_hwTexturePages[tpage][pal];
}

// Dummy texture initilization
void InitHWTextures()
{
	// set loading callbacks
	g_levTextures.SetLoadingCallbacks(InitHWTexturePage, FreeHWTexturePage);
	
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 32; j++)
			g_hwTexturePages[i][j] = g_whiteTexture;
	}
}

//-----------------------------------------------------------------

// extern some vars
extern String					g_levname;
extern String					g_levname_moddir;
extern String					g_levname_texdir;
extern OUT_CITYLUMP_INFO		g_levInfo;
extern CDriverLevelTextures		g_levTextures;
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap*			g_levMap;

FILE* g_levFile = nullptr;

//-------------------------------------------------------
// Perorms level loading and renderer data initialization
//-------------------------------------------------------
bool LoadLevelFile()
{
	g_levFile = fopen(g_levname, "rb");
	if (!g_levFile)
	{
		MsgError("Cannot open %s\n", (char*)g_levname);
		return false;
	}

	CFileStream stream(g_levFile);
	ELevelFormat levFormat = CDriverLevelLoader::DetectLevelFormat(&stream);

	g_levModels.SetModelLoadingCallbacks(CRenderModel::OnModelLoaded, CRenderModel::OnModelFreed);

	// create map accordingly
	if (levFormat >= LEV_FORMAT_DRIVER2_ALPHA16 || levFormat == LEV_FORMAT_AUTODETECT)
		g_levMap = new CDriver2LevelMap();
	else
		g_levMap = new CDriver1LevelMap();

	CDriverLevelLoader loader;
	loader.Initialize(g_levInfo, &g_levTextures, &g_levModels, g_levMap);

	return loader.LoadFromFile(g_levname);
}

//-------------------------------------------------------
// Frees all data
//-------------------------------------------------------
void FreeLevelData()
{
	MsgWarning("Freeing level data ...\n");

	g_levMap->FreeAll();
	g_levTextures.FreeAll();
	g_levModels.FreeAll();

	delete g_levMap;

	fclose(g_levFile);
}

//-------------------------------------------------------
// Render level viewer
//-------------------------------------------------------
void RenderLevelView()
{
	Volume frustumVolume;

	// setup standard camera
	CRenderModel::SetupModelShader();
	SetupCameraViewAndMatrices(g_cameraPosition, g_cameraAngles, frustumVolume);

	GR_SetDepth(1);
	GR_SetCullMode(CULL_FRONT);

	// reset lighting
	CRenderModel::SetupLightingProperties();
	
	if(g_levMap->GetFormat() >= LEV_FORMAT_DRIVER2_ALPHA16)
		DrawLevelDriver2(g_cameraPosition, g_cameraAngles.y, frustumVolume);
	else
		DrawLevelDriver1(g_cameraPosition, g_cameraAngles.y, frustumVolume);
}

float g_cameraDistance = 4.0f;

//-------------------------------------------------------
// Render model viewer
//-------------------------------------------------------
void RenderModelView()
{
	Volume frustumVolume;
	Vector3D forward, right;
	AngleVectors(g_cameraAngles, &forward, &right);

	// setup orbital camera
	CRenderModel::SetupModelShader();
	SetupCameraViewAndMatrices(-forward * g_cameraDistance, g_cameraAngles, frustumVolume);

	CRenderModel::SetupLightingProperties(0.5f, 0.5f);

	GR_SetDepth(1);
	GR_SetCullMode(CULL_FRONT);

	ModelRef_t* ref = g_levModels.GetModelByIndex(g_currentModel);

	if(ref && ref->userData)
	{
		CRenderModel* renderModel = (CRenderModel*)ref->userData;

		renderModel->Draw();

		if (g_displayCollisionBoxes)
			CRenderModel::DrawModelCollisionBox(ref, {0,0,0}, 0.0f);
	}
}

//-------------------------------------------------------------
// Forcefully spools entire level regions and area datas
//-------------------------------------------------------------
void SpoolAllAreaDatas()
{
	Msg("Spooling regions...\n");
	// Open file stream
	FILE* fp = fopen(g_levname, "rb");
	if (fp)
	{
		CFileStream stream(fp);

		SPOOL_CONTEXT spoolContext;
		spoolContext.dataStream = &stream;
		spoolContext.lumpInfo = &g_levInfo;
		spoolContext.models = &g_levModels;
		spoolContext.textures = &g_levTextures;

		int totalRegions = g_levMap->GetRegionsAcross() * g_levMap->GetRegionsDown();
		
		for (int i = 0; i < totalRegions; i++)
		{
			g_levMap->SpoolRegion(spoolContext, i);
		}

		fclose(fp);
	}
	else
		MsgError("Unable to spool area datas!\n");
}

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

char g_modelSearchNameBuffer[64];
void PopulateUIModelNames()
{
	memset(g_modelSearchNameBuffer, 0, sizeof(g_modelSearchNameBuffer));
}

bool g_exportWidget = false;
int g_exportMode = 0;

//-------------------------------------------------------------
// Displays export settings UI
//-------------------------------------------------------------
void DisplayExportUI()
{
	extern bool g_extract_dmodels;
	extern bool g_export_worldUnityScript;
	extern bool g_explode_tpages;
	extern int g_overlaymap_width;
	
	if (ImGui::Begin("Export options", &g_exportWidget))
	{
		ImGui::SetWindowSize(ImVec2(400, 220));

		if(g_exportMode == 0)
		{
			// TODO: Unity export option
			ImGui::Checkbox("Export for Unity Engine", &g_export_worldUnityScript);
			
			if(ImGui::Button("Export world"))
			{
				// idk why, but some regions are bugged while exporting
				SpoolAllAreaDatas();
				
				// export model files as well
				if (g_export_worldUnityScript)
				{
					g_extract_dmodels = false;
					g_explode_tpages = false;
					
					Directory::create(g_levname_moddir);
					Directory::create(g_levname_texdir);

					SaveModelPagesMTL();
					
					ExportAllModels();
					ExportAllTextures();
				}
				
				ExportRegions();
				MsgInfo("Job done!\n");
			}
		}
		else if(g_exportMode == 1)
		{
			ImGui::Checkbox("Only extract as DMODEL", &g_extract_dmodels);

			if (ImGui::Button("Export models"))
			{
				g_export_worldUnityScript = false;
				Directory::create(g_levname_moddir);
				SaveModelPagesMTL();
				ExportAllModels();
				MsgInfo("Job done!\n");
			}
		}
		else if (g_exportMode == 2)
		{
			ImGui::Checkbox("Only extract as DMODEL", &g_extract_dmodels);

			if (ImGui::Button("Export models"))
			{
				g_export_worldUnityScript = false;
				Directory::create(g_levname_moddir);
				SaveModelPagesMTL();
				ExportAllCarModels();
				MsgInfo("Job done!\n");
			}
		}
		else if (g_exportMode == 3)
		{
			ImGui::Checkbox("Extract as TIM files for REDRIVER2", &g_explode_tpages);

			if (ImGui::Button("Export textures"))
			{
				g_export_worldUnityScript = false;
				Directory::create(g_levname_texdir);
				ExportAllTextures();
				MsgInfo("Job done!\n");
			}
		}
		else if (g_exportMode == 4)
		{
			ImGui::InputInt("Image width", &g_overlaymap_width);

			if (ImGui::Button("Export textures"))
			{
				g_export_worldUnityScript = false;
				Directory::create(g_levname_texdir);
				ExportOverlayMap();
				MsgInfo("Job done!\n");
			}
		}

		ImGui::End();
	}
}

// stats counters
extern int g_drawnCells;
extern int g_drawnModels;
extern int g_drawnPolygons;

//-------------------------------------------------------------
// Displays Main menu bar, stats and child windows
//-------------------------------------------------------------
void DisplayUI(float deltaTime)
{
	if(ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Export level..."))
			{
				g_exportWidget = true;
				g_exportMode = 0;
			}

			if (ImGui::MenuItem("Export models..."))
			{
				g_exportWidget = true;
				g_exportMode = 1;
			}

			if (ImGui::MenuItem("Export car models..."))
			{
				g_exportWidget = true;
				g_exportMode = 2;
			}

			if (ImGui::MenuItem("Export textures..."))
			{
				g_exportWidget = true;
				g_exportMode = 3;
			}

			if (ImGui::MenuItem("Export overhead map..."))
			{
				g_exportWidget = true;
				g_exportMode = 4;
			}

			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Mode"))
		{
			if (ImGui::MenuItem("Level viewer"))
				g_viewerMode = 0;
			
			if (ImGui::MenuItem("Model viewer"))
				g_viewerMode = 1;
			
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Level"))
		{
			if (ImGui::MenuItem("Spool all Area Data"))
				SpoolAllAreaDatas();

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

	if(ImGui::Begin("HelpFrame", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiInputTextFlags_NoHorizontalScroll |
		ImGuiWindowFlags_NoSavedSettings | ImGuiColorEditFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		ImGui::SetWindowPos(ImVec2(0, 24));

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "FPS: %d", UpdateFPSCounter(deltaTime));

		if(g_viewerMode == 0)
		{
			ImGui::SetWindowSize(ImVec2(400, 120));
			
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.25f, 1.0f), "Position: X: %d Y: %d Z: %d",
				int(g_cameraPosition.x * ONE_F), int(g_cameraPosition.y * ONE_F), int(g_cameraPosition.z * ONE_F));

			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Draw distance: %d", g_cellsDrawDistance);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Drawn cells: %d", g_drawnCells);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Drawn models: %d", g_drawnModels);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Drawn polygons: %d", g_drawnPolygons);
		}
		else if (g_viewerMode == 1)
		{
			ImGui::SetWindowSize(ImVec2(400, 720));
			
			ModelRef_t* ref = g_levModels.GetModelByIndex(g_currentModel);
			MODEL* model = ref->model;

			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.5f), "Use arrows to change models");

			if(model)
			{
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Polygons: %d", model->num_polys);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Vertices: %d", model->num_vertices);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Point normals: %d", model->num_point_normals);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Instance number: %d", model->instance_number);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Bounding sphere: %d", model->bounding_sphere);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Z bias: %d", model->zBias);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Collision boxes: %d", model->GetCollisionBoxCount());

				if(ref->lowDetailId != 0xFFFF || ref->highDetailId != 0xFFFF)
				{
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "LODs:");
					ImGui::SameLine();

					if(ref->lowDetailId != 0xFFFF)
					{
						ImGui::SameLine();
						if(ImGui::Button("L", ImVec2(14, 14)))
							g_currentModel = ref->lowDetailId;
					}
					
					if(ref->highDetailId != 0xFFFF)
					{
						ImGui::SameLine();
						if (ImGui::Button("H", ImVec2(14, 14)))
							g_currentModel = ref->highDetailId;
					}
				}
				else
				{
					ImGui::Text("");
				}

				int shape_flags = model->shape_flags;

				// shape flags
				ImGui::CheckboxFlags("Lit", &shape_flags, SHAPE_FLAG_LITPOLY); ImGui::SameLine();
				ImGui::CheckboxFlags("BSP", &shape_flags, SHAPE_FLAG_BSPDATA); ImGui::SameLine();
				ImGui::CheckboxFlags("Trans", &shape_flags, SHAPE_FLAG_TRANS); ImGui::SameLine();

				ImGui::CheckboxFlags("Water", &shape_flags, SHAPE_FLAG_WATER); ImGui::SameLine();
				ImGui::CheckboxFlags("Amb2", &shape_flags, SHAPE_FLAG_AMBIENT2);
				ImGui::CheckboxFlags("Amb1", &shape_flags, SHAPE_FLAG_AMBIENT1); ImGui::SameLine();
				
				ImGui::CheckboxFlags("Tile", &shape_flags, SHAPE_FLAG_TILE); ImGui::SameLine();
				ImGui::CheckboxFlags("Shad", &shape_flags, SHAPE_FLAG_SHADOW); ImGui::SameLine();
				ImGui::CheckboxFlags("Alpha", &shape_flags, SHAPE_FLAG_ALPHA); ImGui::SameLine();
				ImGui::CheckboxFlags("Road", &shape_flags, SHAPE_FLAG_ROAD); ImGui::SameLine();
				ImGui::CheckboxFlags("Spr", &shape_flags, SHAPE_FLAG_SPRITE);

				int flags2 = model->flags2;
				ImGui::Spacing();
				ImGui::CheckboxFlags("Junc", &flags2, MODEL_FLAG_JUNC); ImGui::SameLine();
				ImGui::CheckboxFlags("Alley", &flags2, MODEL_FLAG_ALLEY); ImGui::SameLine();
				ImGui::CheckboxFlags("Indrs", &flags2, MODEL_FLAG_INDOORS); ImGui::SameLine();
				ImGui::CheckboxFlags("Chair", &flags2, MODEL_FLAG_CHAIR); ImGui::SameLine();
				ImGui::CheckboxFlags("Barr", &flags2, MODEL_FLAG_BARRIER);
				ImGui::CheckboxFlags("Smsh", &flags2, MODEL_FLAG_SMASHABLE); ImGui::SameLine();
				ImGui::CheckboxFlags("Lamp", &flags2, MODEL_FLAG_LAMP); ImGui::SameLine();
				ImGui::CheckboxFlags("Tree", &flags2, MODEL_FLAG_TREE); ImGui::SameLine();
				ImGui::CheckboxFlags("Grass", &flags2, MODEL_FLAG_GRASS); ImGui::SameLine();
				ImGui::CheckboxFlags("Path", &flags2, MODEL_FLAG_PATH);
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.5f), "MODEL NOT SPOOLED YET");
				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
			}

			ImGui::InputText("Filter", g_modelSearchNameBuffer, sizeof(g_modelSearchNameBuffer));

			ImGuiTextFilter filter(g_modelSearchNameBuffer);

			Array<ModelRef_t*> modelRefs;
			
			for (int i = 0; i < MAX_MODELS; i++)
			{
				ModelRef_t* itemRef = g_levModels.GetModelByIndex(i);
				
				if(!filter.IsActive() && !itemRef->name || itemRef->name && filter.PassFilter(itemRef->name))
					modelRefs.append(itemRef);
			}
			
			if (ImGui::ListBoxHeader("", modelRefs.size(), 30))
			{
				ImGuiListClipper clipper(modelRefs.size(), ImGui::GetTextLineHeightWithSpacing());
				
				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
					{
						ModelRef_t* itemRef = modelRefs[i];
						const bool item_selected = (itemRef->index == g_currentModel);
						
						String item = String::fromPrintf("%d: %s%s", itemRef->index, itemRef->name ? itemRef->name : "", itemRef->model ? "" : "(empty slot)");

						ImGui::PushID(i);

						if (ImGui::Selectable(item, item_selected))
						{
							g_currentModel = itemRef->index;
						}

						ImGui::PopID();
					}
				}
				ImGui::ListBoxFooter();
			}
		}
		ImGui::End();
	}

	if(g_exportWidget)
		DisplayExportUI();
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
			else if (g_holdRight)
			{
				g_cameraDistance += event.motion.yrel * 0.01f;
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
				if (g_viewerMode == 0)
					g_cameraMoveDir.z = (event.type == SDL_KEYDOWN) ? 1.0f : 0.0f;
				else if (g_viewerMode == 1 && (event.type == SDL_KEYDOWN))
				{
					g_currentModel--;
					g_currentModel = MAX(0, g_currentModel);
				}
			}
			else if (nKey == SDL_SCANCODE_DOWN)
			{
				if (g_viewerMode == 0)
					g_cameraMoveDir.z = (event.type == SDL_KEYDOWN) ? -1.0f : 0.0f;
				else if (g_viewerMode == 1 && (event.type == SDL_KEYDOWN))
				{
					g_currentModel++;
					g_currentModel = MIN(MAX_MODELS, g_currentModel);
				}
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

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(g_window);
		ImGui::NewFrame();

		SDLPollEvent();

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
		if (g_viewerMode == 0)
		{
			float cameraSpeedModifier = g_holdShift ? 4.0f : 1.0f;
			
			UpdateCameraMovement(deltaTime, cameraSpeedModifier);
			RenderLevelView();
		}
		else
		{
			RenderModelView();
		}

		DebugOverlay_Draw();

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
int ViewerMain()
{
	if(!GR_Init("OpenDriver2 Level viewer", 1280, 720, 0))
	{
		MsgError("Failed to init graphics!\n");
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(g_window, nullptr);
	ImGui_ImplOpenGL3_Init();

	CRenderModel::InitModelShader();

	DebugOverlay_Init();
	InitHWTextures();

	// Load level file
	if (!LoadLevelFile())
	{
		GR_Shutdown();
		return -1;
	}

	// this is for filtering purposes
	PopulateUIModelNames();

	// loop and stuff
	ViewerMainLoop();

	// free all
	FreeLevelData();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	DebugOverlay_Destroy();
	GR_Shutdown();

	return 0;
}