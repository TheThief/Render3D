#pragma once

#include <type_traits>
#include <algorithm>
#include "math/scalarmath.h"
#include "math/vector_math.h"
#include "array2d.h"
#include "position_vertex.h"

// Depth-only pass
template<typename DepthComparator, typename VertexPositionZ, typename Depth>
void DrawScanline(const int x0i, const int x1i, const float invw_begin, const float invwdelta, const VertexPositionZ vw_position_z_begin, const VertexPositionZ vw_position_zdelta, Depth* __restrict pDepthBufferScanline)
{
	static_assert(DepthComparator::bSetDepth, "DepthComparator must have bSetDepth set");

	pDepthBufferScanline += x0i;

#if __clang__
#pragma clang loop vectorize(enable) vectorize_width(4)
#endif
	for (int x = 0; x < x1i - x0i; ++x)
	{
		float invw2 = invw_begin + x * invwdelta;
		VertexPositionZ vw_position_z2 = vw_position_z_begin + vw_position_zdelta * x;

#if _MSC_VER && !__llvm__
		// Rcp breaks auto-vectorization, but VC++ does not consistently output 1 / invw2 as div*s or rcp*s
		// so we can't use it :(
		const float w = Rcp(invw2);
		auto z = vw_position_z2 * w;

		// this looks odd but allows auto-vectorization of the loop to happen
		pDepthBufferScanline[x] = DepthComparator::Compare(z, pDepthBufferScanline[x]) ? z : pDepthBufferScanline[x];
#else
		// Rcp breaks auto-vectorization :(
		const float w = Rcp(invw2)
		auto z = vw_position_z2 * w;

		if (DepthComparator::Compare(z, pDepthBufferScanline[x]))
		{
			pDepthBufferScanline[x] = z;
		}
#endif
	}
}

template<typename DepthComparator, typename VertexPosition, typename Depth>
void DrawBlock(const VertexPosition& v00w_position, const VertexPosition& v01w_position, const VertexPosition& v10w_position, const VertexPosition& v11w_position, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer)
{
	const float& invw00 = v00w_position.w;
	const float& invw01 = v01w_position.w;
	const float& invw10 = v10w_position.w;
	const float& invw11 = v11w_position.w;
	const float& y00 = v00w_position.y;
	const float& y01 = v01w_position.y;
	const float& y10 = v10w_position.y;
	const float& y11 = v11w_position.y;
	const int y0i = Max(Max((int)ceil(y00), (int)ceil(y01)), ClipBounds.Y0);
	const int y1i = Min(Min((int)ceil(y10), (int)ceil(y11)), ClipBounds.Y1);
	if (y0i >= y1i)
		return;
	const float invy0delta = Rcp(y10 - y00);
	const float invy1delta = Rcp(y11 - y01);
	const float invw0delta = (invw10 - invw00) * invy0delta;
	const float invw1delta = (invw11 - invw01) * invy1delta;
	float invw0 = invw00 + invw0delta * ((float)y0i - y00);
	float invw1 = invw01 + invw1delta * ((float)y0i - y01);
	const VertexPosition v0w_positiondelta = (v10w_position - v00w_position) * invy0delta;
	const VertexPosition v1w_positiondelta = (v11w_position - v01w_position) * invy1delta;
	VertexPosition v0w_position = v00w_position + v0w_positiondelta * ((float)y0i - y00);
	VertexPosition v1w_position = v01w_position + v1w_positiondelta * ((float)y0i - y01);
	for (int y = y0i; y < y1i; y++)
	{
		const float& x0 = v0w_position.x;
		const float& x1 = v1w_position.x;
		const int x0i = std::max((int)ceil(x0), ClipBounds.X0);
		const int x1i = std::min((int)ceil(x1), ClipBounds.X1);

		if (x0i < x1i)
		{
			const float invxdelta = Rcp(x1 - x0);
			const float invwdelta = (invw1 - invw0) * invxdelta;
			float invw = invw0 + invwdelta * ((float)x0i - x0);
			const auto vw_position_zdelta = (v1w_position.z - v0w_position.z) * invxdelta;
			auto vw_position_z = v0w_position.z + vw_position_zdelta * ((float)x0i - x0);

			DrawScanline<DepthComparator>(x0i, x1i, invw, invwdelta, vw_position_z, vw_position_zdelta, DepthBuffer.GetScanlineDataPointer(y));
		}

		invw0 += invw0delta; v0w_position = v0w_position + v0w_positiondelta;
		invw1 += invw1delta; v1w_position = v1w_position + v1w_positiondelta;
	}
}

template<typename DepthComparator, typename Vertex, typename Depth>
void DrawConvexPoly(const Vertex* __restrict vws, int count, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer)
{
	int iLeft = 0;
	for (int i = 1; i < count; i++)
	{
		if (vws[i].Position.y < vws[iLeft].Position.y)
		{
			iLeft = i;
		}
	}
	int iRight = iLeft;
	int iNextLeft = Wrap(iLeft - 1, 0, count);
	int iNextRight = Wrap(iRight + 1, 0, count);
	const Vertex* pv[2][2];
	pv[0][0] = &vws[iLeft];
	pv[0][1] = &vws[iRight];
	pv[1][0] = &vws[iNextLeft];
	pv[1][1] = &vws[iNextRight];

	do 
	{
		if (iNextLeft != iNextRight)
		{
			//// render from either side
			//if (pv[1][0]->Position.x > Lerp(pv[0][1]->Position.x, pv[1][1]->Position.x, Parametize(pv[1][0]->Position.y, pv[0][1]->Position.y, pv[1][1]->Position.y)))
			//{
			//	Swap(pv[1][0], pv[1][1]);
			//}

			DrawBlock<DepthComparator>((*pv[0][0]).Position, (*pv[0][1]).Position, (*pv[1][0]).Position, (*pv[1][1]).Position, ClipBounds, DepthBuffer);

			auto NextLefty = vws[iNextLeft].Position.y;
			auto NextRighty = vws[iNextRight].Position.y;
			// interp on left
			if (NextLefty >= NextRighty)
			{
				iRight = iNextRight;
				iNextRight = Wrap(iRight + 1, 0, count);
				pv[0][1] = pv[1][1];
				pv[1][1] = &vws[iNextRight];
			}
			// interp on right
			else //if (NextLefty <= NextRighty)
			{
				iLeft = iNextLeft;
				iNextLeft = Wrap(iLeft - 1, 0, count);
				pv[0][0] = pv[1][0];
				pv[1][0] = &vws[iNextLeft];
			}

		}
		else
		{
			DrawBlock<DepthComparator>((*pv[0][0]).Position, (*pv[0][1]).Position, (*pv[1][0]).Position, (*pv[1][1]).Position, ClipBounds, DepthBuffer);
			break;
		}
	} while (true);
}

template<typename DepthComparator, typename Globals, typename Vertex, typename Depth>
void DrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer)
{
	// strip all but position data from the verts
	using Position = position_vertex<std::remove_cv_t<std::remove_reference_t<decltype(Vertex::Position)>>>;
	Position Clipped[4];
	int num_clipped = ClipTriangleAgainstPlane(Position(v0.Position), Position(v1.Position), Position(v2.Position), Clipped, Vector4(0,0,1,0));
	if (num_clipped < 3)
	{
		return;
	}
	__assume(num_clipped <= 4);
#if __clang__
#pragma clang loop unroll(full)
#endif
	for (int i = 0; i < num_clipped; i++)
	{
		const float invw = Rcp(Clipped[i].Position.w);
		Clipped[i] = Clipped[i] * invw;
		Clipped[i].Position.w = invw;
	}
	DrawConvexPoly<DepthComparator>(Clipped, num_clipped, ClipBounds, DepthBuffer);
}

template<typename DepthComparator, typename Globals, typename vertex_array, typename index_array, typename Depth>
void DrawIndexedTriList(const Globals& g, const vertex_array& vertices, const index_array& indices, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer)
{
	for (unsigned int x = 0; x + 2 < std::size(indices); x += 3)
	{
		DrawTriangle<DepthComparator>(g, vertices[indices[x + 0]], vertices[indices[x + 1]], vertices[indices[x + 2]], DepthBuffer);
	}
}

template<typename DepthComparator, typename Globals, typename vertex_array, typename index_array, typename Depth>
void DrawIndexedTriList(const Globals& g, const vertex_array& vertices, const index_array& indices, Array2d<Depth>& DepthBuffer)
{
	BoundsRect ClipBounds = BoundsRect(0, DepthBuffer.GetWidth(), 0, DepthBuffer.GetHeight());
	DrawIndexedTriList(g, vertices, indices, ClipBounds, DepthBuffer);
}

template<typename DepthComparator, typename Globals, typename vertex_array, typename index_array, typename Depth>
void DrawIndexedTriStrip(const Globals& g, const vertex_array& vertices, const index_array& indices, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer)
{
	for (unsigned int x = 0; x + 2 < std::size(indices); x++)
	{
		DrawTriangle<DepthComparator>(g, vertices[indices[x + 0]], vertices[indices[x + 1]], vertices[indices[x + 2]], ClipBounds, DepthBuffer);
	}
}

template<typename DepthComparator, typename Globals, typename vertex_array, typename index_array, typename Depth>
void DrawIndexedTriStrip(const Globals& g, const vertex_array& vertices, const index_array& indices, Array2d<Depth>& DepthBuffer)
{
	BoundsRect ClipBounds = BoundsRect(0, DepthBuffer.GetWidth(), 0, DepthBuffer.GetHeight());
	DrawIndexedTriStrip(g, vertices, indices, ClipBounds, DepthBuffer);
}
