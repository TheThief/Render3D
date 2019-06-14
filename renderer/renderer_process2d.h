#pragma once

#include "array2d.h"

#include <type_traits>

template<typename Func, typename Buffer1, typename... BufferNs>
void Process2d(Func&& func, Buffer1&& buffer1, BufferNs&&... bufferNs)
{
	const auto width = buffer1.GetWidth();
	const auto height = buffer1.GetHeight();
	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			func(buffer1(x, y), bufferNs(x, y)...);
		}
	}
}

// Created Output buffer
template<typename Output, typename Func, typename Buffer1, typename... BufferNs>
Output ProcessToNew2d(Func&& func, Buffer1&& buffer1, BufferNs&&... bufferNs)
{
	// todo - uninitialized constructor + placement new + exception safety
	Output output(buffer1.GetWidth(), buffer1.GetHeight());

	auto width = buffer1.GetWidth();
	auto height = buffer1.GetHeight();
	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			output(x, y) = func(buffer1(x, y), bufferNs(x, y)...);
		}
	}
	return output;
}

//////////////////////////////////////////////////////////////////////////

template<typename Pixel, typename PixelAllocator>
typename std::enable_if<std::is_trivially_copy_assignable<Pixel>::value, void>::type Fill2d(Array2d<Pixel, PixelAllocator>& Buffer, const Pixel& p, BoundsRect ClipBounds = BoundsRect())
{
	if (!ClipBounds.IsValid())
	{
		ClipBounds = BoundsRect(0, Buffer.GetWidth(), 0, Buffer.GetHeight());
	}

	for (int y = ClipBounds.Y0; y < ClipBounds.Y1; y++)
	{
		Pixel* const pScanline = Buffer.GetScanlineDataPointer(y);
		Pixel* const pScanlineStart = pScanline + ClipBounds.X0;
		Pixel* const pScanlineEnd = pScanline + ClipBounds.X1;

#if SSE_VERSION >= 2
		static_assert(sizeof(__m128) == 16, "");
		if (sizeof(Pixel) <= 16 &&
			__alignof(Pixel) == sizeof(Pixel))
		{
			void* pPixel = pScanlineStart;
			__m128i m128pixel;
			switch (sizeof(Pixel))
			{
			case 1:
				m128pixel = _mm_set1_epi8((__int8&)p);
				if (-(intptr_t)pScanlineStart & 1)
				{
					*((__int8*&)pPixel)++ = (__int8&)p;
				}
				if (-(intptr_t)pScanlineStart & 2)
				{
					*((__int8*&)pPixel)++ = (__int8&)p;
					*((__int8*&)pPixel)++ = (__int8&)p;
				}
				if (-(intptr_t)pScanlineStart & 4)
				{
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
				}
				if (-(intptr_t)pScanlineStart & 8)
				{
#if _M_X64
					_mm_stream_si64(((__int64*&)pPixel)++, _mm_cvtsi128_si64(m128pixel));
#else
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
#endif
				}
				break;
			case 2:
				m128pixel = _mm_set1_epi16((__int16&)p);
				if (-(intptr_t)pScanlineStart & 2)
				{
					*((__int16*&)pPixel)++ = (__int16&)p;
				}
				if (-(intptr_t)pScanlineStart & 4)
				{
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
				}
				if (-(intptr_t)pScanlineStart & 8)
				{
#if _M_X64
					_mm_stream_si64(((__int64*&)pPixel)++, _mm_cvtsi128_si64(m128pixel));
#else
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
#endif
				}
				break;
			case 4:
				m128pixel = _mm_set1_epi32((__int32&)p);
				if (-(intptr_t)pScanlineStart & 4)
				{
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
				}
				if (-(intptr_t)pScanlineStart & 8)
				{
#if _M_X64
					_mm_stream_si64(((__int64*&)pPixel)++, _mm_cvtsi128_si64(m128pixel));
#else
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
#endif
				}
				break;
			case 8:
#if _M_X64
				m128pixel = _mm_set1_epi64x((__int64&)p);
#else
				check(0);
				m128pixel = _mm_set_epi32(((__int32*)&p)[1],((__int32*)&p)[0], ((__int32*)&p)[1],((__int32*)&p)[0]);
#endif
				if (-(intptr_t)pScanlineStart & 8)
				{
#if _M_X64
					_mm_stream_si64(((__int64*&)pPixel)++, _mm_cvtsi128_si64(m128pixel));
#else
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
					_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
#endif
				}
			case 16:
				m128pixel = (__m128i&)p;
				break;
			default:
				check(0);
			}

			const __m128i* m128ScanlineEnd = (const __m128i*)((intptr_t)pScanlineEnd & ~15);
			__m128i* pm128pixel = (__m128i*)pPixel;

			for (; pm128pixel < m128ScanlineEnd; ++pm128pixel)
			{
				_mm_stream_si128(pm128pixel, m128pixel);
			}
			pPixel = (void*)pm128pixel;

			if (sizeof(Pixel) <= 8 && (intptr_t)pScanlineEnd & 8)
			{
#if _M_X64
				_mm_stream_si64(((__int64*&)pPixel)++, _mm_cvtsi128_si64(m128pixel));
#else
				_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
				_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
#endif
			}
			if (sizeof(Pixel) <= 4 && (intptr_t)pScanlineEnd & 4)
			{
				_mm_stream_si32(((__int32*&)pPixel)++, _mm_cvtsi128_si32(m128pixel));
			}
			if (sizeof(Pixel) <= 2 && (intptr_t)pScanlineEnd & 2)
			{
				*((__int8*&)pPixel)++ = (__int8&)p;
				*((__int8*&)pPixel)++ = (__int8&)p;
			}
			if (sizeof(Pixel) <= 1 && (intptr_t)pScanlineEnd & 1)
			{
				*((__int8*&)pPixel)++ = (__int8&)p;
			}

			check(pPixel == pScanlineEnd);
		}
		else
#endif
		std::fill(pScanlineStart, pScanlineEnd, p);

		//for (Pixel* pPixel = pScanline; pPixel < pScanlineEnd; ++pPixel)
		//{
		//	*pPixel = p;
		//}
	}
}

template<typename Pixel, typename PixelAllocator>
typename std::enable_if<!std::is_trivially_copy_assignable<Pixel>::value, void>::type Fill2d(Array2d<Pixel, PixelAllocator>& Buffer, const Pixel& p, BoundsRect ClipBounds = BoundsRect())
{
	if (!ClipBounds.IsValid())
	{
		ClipBounds = BoundsRect(0, Buffer.GetWidth(), 0, Buffer.GetHeight());
	}

	for (int y = ClipBounds.Y0; y < ClipBounds.Y1; y++)
	{
		Pixel* const pScanline = Buffer.GetScanlineDataPointer(y);
		Pixel* const pScanlineStart = pScanline + ClipBounds.X0;
		Pixel* const pScanlineEnd = pScanline + ClipBounds.X1;

		std::fill(pScanlineStart, pScanlineEnd, p);
		//for (Pixel* pPixel = pScanline; pPixel < pScanlineEnd; ++pPixel)
		//{
		//	*pPixel = p;
		//}
	}
}
