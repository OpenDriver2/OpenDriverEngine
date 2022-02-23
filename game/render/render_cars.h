#ifndef RENDER_CARS_H
#define RENDER_CARS_H

class CCar;
struct TexDetailInfo_t;

class CRender_Cars
{
public:
	static void		InitCarRender();
	static void		TerminateCarRender();

	static void		MangleWheelModel(struct MODEL* model);

	static void		DrawCars(Array<CCar*>& cars);

private:
	static void				AddCarShadow(CMeshBuilder& meshBuilder, CCar* car);
	static GrVAO*			carShadowVAO;
	static Vector4D			carShadowUV;
	static TexDetailInfo_t* carShadowDetail;
};

#endif // RENDER_CARS_H