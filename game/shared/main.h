#ifndef DRIVER_LEVEL_H
#define DRIVER_LEVEL_H

#include "routines/models.h"
#include "routines/textures.h"
#include "routines/regions.h"
#include "routines/level.h"

#include "math/Matrix.h"

//----------------------------------------------------------

#define EXPORT_SCALING			(1.0f / ONE_F)

//----------------------------------------------------------

// extern some vars
extern OUT_CITYLUMP_INFO		g_levInfo;
extern CDriverLevelTextures		g_levTextures;
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap*			g_levMap;


#endif