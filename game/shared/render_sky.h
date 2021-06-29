#ifndef RENDER_SKY_H
#define RENDER_SKY_H

// TODO: CSkyRenderer

extern UV		g_skytexuv[28];
extern uint8	g_HorizonLookup[4][4];
extern uint8	g_HorizonTextures[40];

// Initialize sky texture and UVs
bool InitSky(int skyNumber);

// Destroy sky and UVs
void DestroySky();

// Renders a sky
void RenderSky();

#endif // RENDER_SKY_H