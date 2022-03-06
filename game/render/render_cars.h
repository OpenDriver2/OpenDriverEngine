#ifndef RENDER_CARS_H
#define RENDER_CARS_H

class CCar;
struct TexDetailInfo_t;

class CRender_Cars
{
public:
	static void		InitRender();
	static void		TerminateRender();

	static void		MangleWheelModel(struct MODEL* model);

	static void		DrawCars(Array<CCar*>& cars);

private:
	static void				AddCarShadow(CMeshBuilder& meshBuilder, CCar* car);
	static GrVAO*			ShadowVAO;
	static Vector4D			ShadowUV;
	static TexDetailInfo_t* ShadowDetail;
};

#endif // RENDER_CARS_H