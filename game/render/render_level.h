#ifndef RENDERLEVEL_H
#define RENDERLEVEL_H

class Volume;
struct DRAWABLE;

struct LevelRenderProps
{
	ColorRGB ambientColor{ 0.95f, 0.9f, 1.0f };
	ColorRGB lightColor{ 0.95f, 0.9f, 0.5f };

	float nightAmbientScale{ 0.35f };
	float nightLightScale{ 0.0f };

	float ambientScale{ 1.0f };
	float lightScale{ 1.0f };

	bool nightMode { false };

	bool noLod { false };

	// debug stuff
	bool displayCollisionBoxes{ false };
	bool displayHeightMap{ false };
	bool displayAllCellLevels{ true };	// TODO: transform to list of drawn cells
};

class CRender_Level
{
public:
	static LevelRenderProps RenderProps;

	static void	InitRender();
	static void	TerminateRender();

	static void DrawMap(const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume);

	static void DrawObject(const DRAWABLE& drawable, const Vector3D& cameraPos, const Volume& frustrumVolume, bool buildingLighting);

	static void DrawCellObject(
		const CELL_OBJECT& co, 
		const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume, 
		bool buildingLighting);

private:
	static void DrawCellObject(
		const CELL_OBJECT& co, const Vector3D& position, const ModelRef_t* ref, 
		float cameraAngleY, bool buildingLighting);

	static void DrawObjectShadow(CMeshBuilder& shadowMesh, const Matrix3x3& shadowMat, const ModelRef_t* ref, const Vector3D& position, float distance);

	static GrVAO* ShadowVAO;
};



#endif // RENDERLEVEL_H