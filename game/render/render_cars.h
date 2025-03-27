#pragma once

class CViewParams;
class CMeshBuilder;
class CCar;
struct TexDetailInfo_t;

class IGPUBuffer;
using IGPUBufferPtr = CRefPtr<IGPUBuffer>;

class CRender_Cars
{
public:
	static void		Init();
	static void		Terminate();

	static void		MangleWheelModel(struct MODEL* model);

	static void		DrawCars(ArrayCRef<CCar*> cars, const CViewParams& view);

private:
	static void				AddCarShadow(CMeshBuilder& meshBuilder, CCar* car, float distance);
	static IGPUBufferPtr	ShadowGeomBuffer;
	static Vector4D			ShadowUV;
	static TexDetailInfo_t* ShadowDetail;
};
