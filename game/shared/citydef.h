#ifndef CITYDEF_H
#define CITYDEF_H

enum ECityGameType
{
	CityType_Day = 0,
	CityType_Night,
	CityType_MP_Day,
	CityType_MP_Night
};

// HUD/overlay map information
struct OverlayMapDef_t
{
	int x_offset, y_offset;

	int width, height;
	int scale;

	uint8 toptile, dummyImage;
};

// this is read from JSON
struct CityDef_t
{
	int					number;		// 0 = Chicago, 1 = Havana, 2 = Vegas, 3 = Rio for Driver2
	int					version;	// 1 = Driver 1, 2 = Driver 2

	String				name;
	String				skyPath;	// sky texture path

	String				levPath[4];	// ECityGameType - Day, Night, MP Day, MP Night

	OverlayMapDef_t		map;
};

#endif // CITYDEF_H