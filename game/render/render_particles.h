#pragma once

class CPointParticleRender
{
public:
	void Init();
	void Terminate();

	void Render();

protected:

	struct PointParticle
	{
		TextureID texture;
		Vector3D position;
		Vector4D uvs;
		ColorRGB color;
		float rotation;
		float size;
	};

	Array<PointParticle>	m_particles;
	GrVAO*					m_renderPoints{ nullptr };
};