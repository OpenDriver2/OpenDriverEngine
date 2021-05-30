#ifndef RENDERLEVEL_H
#define RENDERLEVEL_H

class Volume;
void DrawLevelDriver1(const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume);
void DrawLevelDriver2(const Vector3D& cameraPos, float cameraAngleY, const Volume& frustrumVolume);

#endif // RENDERLEVEL_H