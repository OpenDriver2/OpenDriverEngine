#ifndef RENDERLEVEL_H
#define RENDERLEVEL_H

class Volume;

/*
// TODO: lua_stuff.h
#define define_prop_get(type, var) \
	type& get_##var() {return var;}\

#define define_prop_set(type, var) \
	void set_##var(type newVal) {var = newVal;}

#define define_property(type, var, defaultValue)\
	type var{ defaultValue }; \
	define_prop_get(type, var) \
	define_prop_set(type,var)

#define define_property_readonly(type, var, defaultValue)\
	type var{ defaultValue }; \
	define_prop_get(type, var)

#define define_property_writeonly(type, var, defaultValue)\
	type var{ defaultValue }; \
	define_prop_set(type, var)

#define sol_property(classname, var) sol::property(&classname::get_##var, &classname::set_##var)
#define sol_property_readonly(classname, var) sol::property(&classname::get_##var)
#define sol_property_writeonly(classname, var) sol::property(nullptr, &classname::set_##var)

struct LevelRenderProps
{
	define_property(float, nightAmbientScale, 0.35f );
	define_property(float, nightLightScale, 0.0f );

	define_property(float, ambientScale, 1.0f );
	define_property(float, lightScale, 1.0f );

	define_property(bool, nightMode, false );

	define_property(bool, noLod, false);

	// debug stuff
	define_property(bool, displayCollisionBoxes, false );
	define_property(bool, displayHeightMap, false );
	define_property(bool, displayAllCellLevels, true );	// TODO: transform to list of drawn cells
};
*/

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

extern LevelRenderProps g_levRenderProps;

void DrawMap(const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume);

void DrawCellObject(const CELL_OBJECT& co, const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume, bool buildingLighting);

#endif // RENDERLEVEL_H