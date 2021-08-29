#ifndef BCOLL3D_H
#define BCOLL3D_H

class CCar;

int CarCarCollision3(CCar* c0, CCar* c1, int& depth, struct VECTOR_NOPAD& where, struct VECTOR_NOPAD& normal);

#endif // BCOLL3D_H