//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: Geometry tools - not so inline
//////////////////////////////////////////////////////////////////////////////////

template <typename T>
void ComputeTriangleNormal( const TVec3D<T> &v0, const TVec3D<T> &v1, const TVec3D<T> &v2, TVec3D<T>& normal)
{
	normal = normalize(cross(v2 - v1, v0 - v1));
}

template <typename T, typename T2>
void ComputeTriangleTBN( const TVec3D<T> &v0, const TVec3D<T> &v1, const TVec3D<T> &v2, const TVec2D<T2> &t0, const TVec2D<T2> &t1, const TVec2D<T2> &t2, TVec3D<T>& normal, TVec3D<T>& tangent, TVec3D<T>& binormal)
{
	//Calculate vectors along polygon sides
	TVec3D<T> side0 = v0-v1;
	TVec3D<T> side1 = v2-v1;

	//Calculate normal
	normal = cross(side1, side0);
	normal = normalize(normal);

	//Calculate s tangent
	float deltaT0 = t0.y - t1.y;
	float deltaT1 = t2.y - t1.y;

	TVec3D<T> tempSTangent = deltaT1 * side0 - deltaT0 * side1;
	tempSTangent = normalize(tempSTangent);

	//Calculate t tangent
	float deltaS0 = t0.x - t1.x;
	float deltaS1 = t2.x - t1.x;
	TVec3D<T> tempTTangent = deltaS1 * side0 - deltaS0 * side1;
	tempTTangent = normalize(tempTTangent);

	//reverse tangents if necessary
	TVec3D<T> tangentCross = cross(tempSTangent, tempTTangent);
	if(dot(tangentCross, normal)<0.0f)
	{
		tempSTangent = -tempSTangent;
		tempTTangent = -tempTTangent;
	}

	//Output results
	tangent = tempSTangent;
	binormal = tempTTangent;
}

template <typename T>
float ComputeTriangleArea( const TVec3D<T> &v0, const TVec3D<T> &v1, const TVec3D<T> &v2 )
{
	// FIXME: this is slow (4 sqrt's), but whatever

	TVec3D<T> e0 = v0-v1;
	TVec3D<T> e1 = v1-v2;
	TVec3D<T> e2 = v2-v0;

	float l0 = length(v0);
	float l1 = length(v1);
	float l2 = length(v2);

	float p = (l0+l1+l2) * 0.5f;

	return sqrt( p * (p-l0) * (p-l1) * (p-l2) );
}