#ifndef BCOLL3D_H
#define BCOLL3D_H

class CCar;

struct CRET3D
{
	VECTOR_NOPAD location;
	VECTOR_NOPAD normal;
	int depth;
};

bool collided3d(CCar* cp0, CCar* cp1, CRET3D& least);

#endif // BCOLL3D_H