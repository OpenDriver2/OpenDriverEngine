#ifndef WORLD_H
#define WORLD_H

#include "routines/models.h"
#include "routines/textures.h"
#include "routines/regions.h"
#include "routines/level.h"

#include "math/Matrix.h"

// extern some vars
extern OUT_CITYLUMP_INFO		g_levInfo;
extern CDriverLevelTextures		g_levTextures;
extern CDriverLevelModels		g_levModels;
extern CBaseLevelMap* g_levMap;

int ViewerMain();

#endif // WORLD_H