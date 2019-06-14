#pragma once

#include "math/scalarmath.h"
#include "math/vector_math.h"

#define RENDERER_USE_RCP 1

struct BoundsRect
{
	int X0; // X0 <= X < X1
	int X1;
	int Y0; // Y0 <= Y < Y1
	int Y1;

	BoundsRect()
		: X0(INT_MAX), X1(INT_MIN), Y0(INT_MAX), Y1(INT_MIN)
	{
	}

	BoundsRect(int X0, int X1, int Y0, int Y1)
		: X0(X0), X1(X1), Y0(Y0), Y1(Y1)
	{
	}

	bool IsValid()
	{
		return X0 < X1;
	}
};

template<typename Vertex>
inline void ClipTriangleSegmentAgainstPlane(const Vertex& In0, const Vertex& In1, float dot1, float dot2, int& Verts, Vertex (&Out)[4])
{
	float frac;
	switch(ClipLineAgainstPlane(dot1, dot2, frac))
	{
	case CR_EntirelyInside:
		Out[Verts] = In0;
		Verts++;
		break;
	case CR_EntirelyOutside:
		break;
	case CR_Leaving:
		Out[Verts] = In0;
		Verts++;
		Out[Verts] = Lerp(In0, In1, frac);
		Verts++;
		break;
	case CR_Entering:
		Out[Verts] = Lerp(In0, In1, frac);
		Verts++;
		break;
	}
}

template<typename Vertex>
int ClipTriangleAgainstPlane(const Vertex& In0, const Vertex& In1, const Vertex& In2, Vertex (&Out)[4], const Vector4& Plane)
{
	float dot0 = PointToPlaneSpace(In0.Position, Plane);
	float dot1 = PointToPlaneSpace(In1.Position, Plane);
	float dot2 = PointToPlaneSpace(In2.Position, Plane);

	int Verts = 0;
	ClipTriangleSegmentAgainstPlane(In0, In1, dot0, dot1, Verts, Out);
	ClipTriangleSegmentAgainstPlane(In1, In2, dot1, dot2, Verts, Out);
	ClipTriangleSegmentAgainstPlane(In2, In0, dot2, dot0, Verts, Out);
	return Verts;
}

#if !RENDERER_USE_RCP
#define Rcp(x) (1.0f / (x))
#endif

#include "renderer_draw_shaded.h"
#include "renderer_draw_depthonly.h"

#if !RENDERER_USE_RCP
#undef Rcp
#endif
