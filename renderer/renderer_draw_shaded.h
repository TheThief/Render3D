#pragma once

#include "math/scalarmath.h"
#include "math/vector_math.h"
#include "array2d.h"

// C++0x Multibuffer
//template<typename PixelFunctor, typename Globals, typename Vertex, typename Pixel, typename... Pixels>
//void DrawScanline(const Globals& g, const int x0i, const int x1i, float invw, const float invwdelta, Vertex& vw, const Vertex& vwdelta, Pixel* pBufferScanline, Pixels*... pBuffersScanlines)
//{
//	for (int x = x0i; x < x1i; x++)
//	{
//		const float w = Rcp(invw);
//		const Vertex v = vw * w;
//		//if (y == y0i || y == y1i-1)
//		//{
//		//	pBufferScanline[x] = ARGB(255,0,0);
//		//}
//		//else if (x == x0i || x == x1i-1)
//		//{
//		//	pBufferScanline[x] = ARGB(255,255,0);
//		//}
//		//else
//		PixelFunctor::Func(g, v, pBufferScanline[x], pBuffersScanlines[x]...);
//
//		invw += invwdelta; vw = vw + vwdelta;
//	}
//}
//
//template<typename PixelFunctor, typename Globals, typename Vertex, typename Pixel, typename... Pixels>
//void DrawBlock(const Globals& g, const Vertex& v00w, const Vertex& v01w, const Vertex& v10w, const Vertex& v11w, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
//{
//	const float& invw00 = v00w.Position.w;
//	const float& invw01 = v01w.Position.w;
//	const float& invw10 = v10w.Position.w;
//	const float& invw11 = v11w.Position.w;
//	const float& y00 = v00w.Position.y;
//	const float& y01 = v01w.Position.y;
//	const float& y10 = v10w.Position.y;
//	const float& y11 = v11w.Position.y;
//	const int y0i = Max(Max((int)ceil(y00), (int)ceil(y01)), 0);
//	const int y1i = Min(Min((int)ceil(y10), (int)ceil(y11)), (int)Buffer.GetHeight());
//	if (y0i >= y1i)
//		return;
//	const float invy0delta = Rcp(y10 - y00);
//	const float invy1delta = Rcp(y11 - y01);
//	const float invw0delta = (invw10 - invw00) * invy0delta;
//	const float invw1delta = (invw11 - invw01) * invy1delta;
//	float invw0 = invw00 + invw0delta * ((float)y0i - y00);
//	float invw1 = invw01 + invw1delta * ((float)y0i - y01);
//	const Vertex v0wdelta = (v10w - v00w) * invy0delta;
//	const Vertex v1wdelta = (v11w - v01w) * invy1delta;
//	Vertex v0w = v00w + v0wdelta * ((float)y0i - y00);
//	Vertex v1w = v01w + v1wdelta * ((float)y0i - y01);
//	for (int y = y0i; y < y1i; y++)
//	{
//		const float& x0 = v0w.Position.x;
//		const float& x1 = v1w.Position.x;
//		const int x0i = Max((int)ceil(x0), 0);
//		const int x1i = Min((int)ceil(x1), (int)Buffer.GetWidth());
//
//		if (x0i < x1i)
//		{
//			const float invxdelta = Rcp(x1 - x0);
//			const float invwdelta = (invw1 - invw0) * invxdelta;
//			float invw = invw0 + invwdelta * ((float)x0i - x0);
//			const Vertex vwdelta = (v1w - v0w) * invxdelta;
//			Vertex vw = v0w + vwdelta * ((float)x0i - x0);
//
//			DrawScanline<PixelFunctor>(g, x0i, x1i, invw, invwdelta, vw, vwdelta, Buffer.GetScanlineDataPointer(y), Buffers.GetScanlineDataPointer(y)...);
//		}
//
//		invw0 += invw0delta; v0w = v0w + v0wdelta;
//		invw1 += invw1delta; v1w = v1w + v1wdelta;
//	}
//}
//
//template<typename PixelFunctor, typename Globals, typename Vertex, typename Pixel, typename... Pixels>
//void DrawConvexPoly(const Globals& g, const Vertex* v, int count, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
//{
//	int iLeft = 0;
//	for (int i = 1; i < count; i++)
//	{
//		if (v[i].Position.y < v[iLeft].Position.y)
//		{
//			iLeft = i;
//		}
//	}
//	int iRight = iLeft;
//	int iNextLeft = Wrap(iLeft - 1, 0, count);
//	int iNextRight = Wrap(iRight + 1, 0, count);
//	const Vertex* pv[2][2];
//	pv[0][0] = &v[iLeft];
//	pv[0][1] = &v[iRight];
//	pv[1][0] = &v[iNextLeft];
//	pv[1][1] = &v[iNextRight];
//
//	do 
//	{
//		if (iNextLeft != iNextRight)
//		{
//			//// render from either side
//			//if (pv[1][0]->Position.x > Lerp(pv[0][1]->Position.x, pv[1][1]->Position.x, Parametize(pv[1][0]->Position.y, pv[0][1]->Position.y, pv[1][1]->Position.y)))
//			//{
//			//	Swap(pv[1][0], pv[1][1]);
//			//}
//
//			DrawBlock<PixelFunctor>(g, *pv[0][0], *pv[0][1], *pv[1][0], *pv[1][1], Buffer, Buffers...);
//
//			// interp on left
//			if (v[iNextLeft].Position.y > v[iNextRight].Position.y)
//			{
//				iRight = iNextRight;
//				iNextRight = Wrap(iRight + 1, 0, count);
//				pv[0][1] = pv[1][1];
//				pv[1][1] = &v[iNextRight];
//			}
//			// interp on right
//			else //if (v[iNextLeft].Position.y < v[iNextRight].Position.y)
//			{
//				iLeft = iNextLeft;
//				iNextLeft = Wrap(iLeft - 1, 0, count);
//				pv[0][0] = pv[1][0];
//				pv[1][0] = &v[iNextLeft];
//			}
//			// identical y (unlikely)
//			//else
//			//{
//			//	iLeft = iNextLeft;
//			//	iRight = iNextRight;
//			//}
//		}
//		else
//		{
//			DrawBlock<PixelFunctor>(g, *pv[0][0], *pv[0][1], *pv[1][0], *pv[1][1], Buffer, Buffers...);
//			break;
//		}
//	} while (true);
//}
//
//template<typename PixelFunctor, typename Globals, typename Vertex, typename Pixel, typename... Pixels>
//void DrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
//{
//	Vertex Clipped[4];
//	int Verts = ClipTriangleAgainstPlane(v0, v1, v2, Clipped, Vector4(0,0,1,0));
//	if (Verts < 3)
//	{
//		return;
//	}
//	for (int i = 0; i < Verts; i++)
//	{
//		const float invw = Rcp(Clipped[i].Position.w);
//		Clipped[i] = Clipped[i] * invw;
//		Clipped[i].Position.w = invw;
//	}
//	DrawConvexPoly<PixelFunctor>(g, Clipped, Verts, Buffer, Buffers...);
//}
//
//template<typename PixelFunctor, typename Globals, typename Vertex, typename Index, typename Pixel, typename... Pixels>
//void DrawIndexedTriList(const Globals& g, const Array<Vertex>& Verts, const Array<Index>& Indices, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
//{
//	for (unsigned int x = 0; x + 2 < Indices.GetSize(); x += 3)
//	{
//		DrawTriangle<PixelFunctor>(g, Verts(Indices(x+0)), Verts(Indices(x+1)), Verts(Indices(x+2)), Buffer, Buffers...);
//	}
//}
//
//template<typename PixelFunctor, typename Globals, typename Vertex, typename Index, typename Pixel, typename... Pixels>
//void DrawIndexedTriStrip(const Globals& g, const Array<Vertex>& Verts, const Array<Index>& Indices, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
//{
//	for (unsigned int x = 0; x + 2 < Indices.GetSize(); x++)
//	{
//		DrawTriangle<PixelFunctor>(g, Verts(Indices(x+0)), Verts(Indices(x+1)), Verts(Indices(x+2)), Buffer, Buffers...);
//	}
//}

// C++0x Depth Buffer + Multibuffer
template<typename DepthComparator, typename PixelFunctor, typename Globals, typename Vertex, typename Depth, typename Pixel, typename... Pixels>
void DrawScanline(const Globals& g, const int x0i, const int x1i, float invw, const float invwdelta, Vertex& vw, const Vertex& vwdelta, Depth* __restrict pDepthBufferScanline, Pixel* __restrict pBufferScanline, Pixels* __restrict ... pBuffersScanlines)
{
	//for (int x = x0i; x < x1i; x++)
	//{
	//	const float w = Rcp(invw);
	//	auto z = vw.Position.z * w;
	//	if (DepthComparator::Compare(z, pDepthBufferScanline[x]))
	//	{
	//		Vertex v = vw * w;
	//		v.Position = vw.Position;
	//		//if (y == y0i || y == y1i-1)
	//		//{
	//		//	pBufferScanline[x] = ARGB(255,0,0);
	//		//}
	//		//else if (x == x0i || x == x1i-1)
	//		//{
	//		//	pBufferScanline[x] = ARGB(255,255,0);
	//		//}
	//		//else
	//		PixelFunctor::Func(g, v, pBufferScanline[x], pBuffersScanlines[x]...);

	//		if (DepthComparator::bSetDepth)
	//		{
	//			pDepthBufferScanline[x] = z;
	//		}
	//	}

	//	invw += invwdelta; vw = vw + vwdelta;
	//}

	for (int x = 0; x < x1i - x0i; x++)
	{
		float invw2 = invw + invwdelta * x;
		Vertex vw2 = vw + vwdelta * (float)x;

#if __llvm__
		// Rcp breaks auto-vectorization :(
		const float w = 1 / invw2;
#else
		const float w = Rcp(invw2);
#endif
		auto z = vw2.Position.z * w;
		if (DepthComparator::Compare(z, (pDepthBufferScanline + x0i)[x]))
		{
			if (DepthComparator::bSetDepth)
			{
				(pDepthBufferScanline + x0i)[x] = z;
			}

			Vertex v = vw2 * w;
			v.Position = vw2.Position;

			//if (x == 0 || x == x1i-x0i-1)
			//{
			//	(pBufferScanline + x0i)[x] = ARGB(255, 255, 0);
			//}
			//else
			PixelFunctor::Func(g, v, (pBufferScanline + x0i)[x], (pBuffersScanlines + x0i)[x]...);
		}
	}
}

template<typename DepthComparator, typename PixelFunctor, typename Globals, typename Vertex, typename Depth, typename Pixel, typename... Pixels>
void DrawBlock(const Globals& g, const Vertex& v00w, const Vertex& v01w, const Vertex& v10w, const Vertex& v11w, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
{
	const float& invw00 = v00w.Position.w;
	const float& invw01 = v01w.Position.w;
	const float& invw10 = v10w.Position.w;
	const float& invw11 = v11w.Position.w;
	const float& y00 = v00w.Position.y;
	const float& y01 = v01w.Position.y;
	const float& y10 = v10w.Position.y;
	const float& y11 = v11w.Position.y;
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
	const Vertex v0wdelta = (v10w - v00w) * invy0delta;
	const Vertex v1wdelta = (v11w - v01w) * invy1delta;
	Vertex v0w = v00w + v0wdelta * ((float)y0i - y00);
	Vertex v1w = v01w + v1wdelta * ((float)y0i - y01);
	for (int y = y0i; y < y1i; y++)
	{
		const float& x0 = v0w.Position.x;
		const float& x1 = v1w.Position.x;
		const int x0i = Max((int)ceil(x0), ClipBounds.X0);
		const int x1i = Min((int)ceil(x1), ClipBounds.X1);

		if (x0i < x1i)
		{
			const float invxdelta = Rcp(x1 - x0);
			const float invwdelta = (invw1 - invw0) * invxdelta;
			float invw = invw0 + invwdelta * ((float)x0i - x0);
			const Vertex vwdelta = (v1w - v0w) * invxdelta;
			Vertex vw = v0w + vwdelta * ((float)x0i - x0);

			DrawScanline<DepthComparator, PixelFunctor>(g, x0i, x1i, invw, invwdelta, vw, vwdelta, DepthBuffer.GetScanlineDataPointer(y), Buffer.GetScanlineDataPointer(y), Buffers.GetScanlineDataPointer(y)...);
		}

		invw0 += invw0delta; v0w = v0w + v0wdelta;
		invw1 += invw1delta; v1w = v1w + v1wdelta;
	}
}

template<typename DepthComparator, typename PixelFunctor, typename Globals, typename Vertex, typename Depth, typename Pixel, typename... Pixels>
void DrawConvexPoly(const Globals& g, const Vertex* __restrict vws, int count, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
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

			DrawBlock<DepthComparator, PixelFunctor>(g, *pv[0][0], *pv[0][1], *pv[1][0], *pv[1][1], ClipBounds, DepthBuffer, Buffer, Buffers...);

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
			DrawBlock<DepthComparator, PixelFunctor>(g, *pv[0][0], *pv[0][1], *pv[1][0], *pv[1][1], ClipBounds, DepthBuffer, Buffer, Buffers...);
			break;
		}
	} while (true);
}

template<typename DepthComparator, typename PixelFunctor, typename Globals, typename Vertex, typename Depth, typename Pixel, typename... Pixels>
void DrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
{
	Vertex Clipped[4];
	int Verts = ClipTriangleAgainstPlane(v0, v1, v2, Clipped, Vector4(0,0,1,0)); // Vector4(0,0,-1,-0.1) ??
	if (Verts < 3)
	{
		return;
	}
	__assume(Verts <= 4);
#if __clang__
#pragma clang loop unroll(full)
#endif
	for (int i = 0; i < Verts; i++)
	{
		const float invw = Rcp(Clipped[i].Position.w);
		Clipped[i] = Clipped[i] * invw;
		Clipped[i].Position.w = invw;
	}
	DrawConvexPoly<DepthComparator, PixelFunctor>(g, Clipped, Verts, ClipBounds, DepthBuffer, Buffer, Buffers...);
}

template<typename DepthComparator, typename PixelFunctor, typename Globals, typename vertex_array, typename index_array, typename Depth, typename Pixel, typename... Pixels>
void DrawIndexedTriList(const Globals& g, const vertex_array& vertices, const index_array& indices, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer)
{
	for (unsigned int x = 0; x + 2 < std::size(indices); x += 3)
	{
		DrawTriangle<DepthComparator, PixelFunctor>(g, vertices[indices[x + 0]], vertices[indices[x + 1]], vertices[indices[x + 2]], ClipBounds, DepthBuffer);
	}
}

template<typename DepthComparator, typename PixelFunctor, typename Globals, typename vertex_array, typename index_array, typename Depth, typename Pixel, typename... Pixels>
void DrawIndexedTriList(const Globals& g, const vertex_array& vertices, const index_array& indices, Array2d<Depth>& DepthBuffer, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
{
	BoundsRect ClipBounds = BoundsRect(0, DepthBuffer.GetWidth(), 0, DepthBuffer.GetHeight());
	DrawIndexedTriList<DepthComparator, PixelFunctor>(g, vertices, indices, ClipBounds, DepthBuffer, Buffer, Buffers...);
}

template<typename DepthComparator, typename PixelFunctor, typename Globals, typename vertex_array, typename index_array, typename Depth, typename Pixel, typename... Pixels>
void DrawIndexedTriStrip(const Globals& g, const vertex_array& vertices, const index_array& indices, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer)
{
	for (unsigned int x = 0; x + 2 < std::size(indices); x++)
	{
		DrawTriangle<DepthComparator, PixelFunctor>(g, vertices[indices[x + 0]], vertices[indices[x + 1]], vertices[indices[x + 2]], ClipBounds, DepthBuffer);
	}
}

template<typename DepthComparator, typename PixelFunctor, typename Globals, typename vertex_array, typename index_array, typename Depth, typename Pixel, typename... Pixels>
void DrawIndexedTriStrip(const Globals& g, const vertex_array& vertices, const index_array& indices, Array2d<Depth>& DepthBuffer, Array2d<Pixel>& Buffer, Array2d<Pixels>&... Buffers)
{
	BoundsRect ClipBounds = BoundsRect(0, DepthBuffer.GetWidth(), 0, DepthBuffer.GetHeight());
	DrawIndexedTriStrip<DepthComparator, PixelFunctor>(g, vertices, indices, ClipBounds, DepthBuffer, Buffer, Buffers...);
}
