#pragma once

#include "globaldefs.h"
#include "scalarmath.h"

#ifdef RGB
#undef RGB
#endif

struct RGB
{
	byte B;
	byte G;
	byte R;

	RGB() :
		B(255),
		G(0),
		R(255)
	{
	}

	RGB(byte R, byte G, byte B) :
		B(B),
		G(G),
		R(R)
	{
	}

	RGB operator +(const RGB& rhs) const
	{
		return RGB(R + rhs.R, G + rhs.G, B + rhs.B);
	}

	RGB operator -(const RGB& rhs) const
	{
		return RGB(R - rhs.R, G - rhs.G, B - rhs.B);
	}

	template<typename type>
	RGB operator *(const type& rhs) const
	{
		return RGB((byte)(R * rhs), (byte)(G * rhs), (byte)(B * rhs));
	}

	template<typename type>
	const RGB& operator *=(const type& rhs)
	{
		R = (byte)(R * rhs);
		G = (byte)(G * rhs);
		B = (byte)(B * rhs);
		return *this;
	}

	friend RGB Lerp(const RGB& X, const RGB& Y, float Alpha)
	{
		RGB Result;
		Result.R = Lerp(X.R, Y.R, Alpha);
		Result.G = Lerp(X.G, Y.G, Alpha);
		Result.B = Lerp(X.B, Y.B, Alpha);
		return Result;
	}
};

struct ARGB
{
	union
	{
		struct 
		{
			byte B;
			byte G;
			byte R;
			byte A;
		};
		unsigned int dwColour;
	};

	ARGB() :
		B(255),
		G(0),
		R(255),
		A(255)
	{
	}

	ARGB(byte R, byte G, byte B, byte A = 255) :
		B(B),
		G(G),
		R(R),
		A(A)
	{
	}

	ARGB(unsigned int dw) :
		dwColour(dw)
	{
	}

	ARGB(const RGB& RGB, byte A = 255) :
		B(RGB.B),
		G(RGB.G),
		R(RGB.R),
		A(A)
	{
	}

	ARGB& operator=(const ARGB& rhs) = default;
	//{
	//	dwColour = rhs.dwColour;
	//	return *this;
	//}

	friend ARGB Lerp(const ARGB& X, const ARGB& Y, float Alpha)
	{
		ARGB Result;
		Result.R = Lerp(X.R, Y.R, Alpha);
		Result.G = Lerp(X.G, Y.G, Alpha);
		Result.B = Lerp(X.B, Y.B, Alpha);
		Result.A = Lerp(X.A, Y.A, Alpha);
		return Result;
	}

	explicit operator unsigned int()
	{
		return dwColour;
	}
};

struct alignstruct(16) fRGBA
{
	union
	{
		struct
		{
			float R;
			float G;
			float B;
			float A;
		};
#if SSE_VERSION >= 2
		__m128 mm;
#endif
	};

	fRGBA() :
		R(1),
		G(0),
		B(1),
		A(1)
	{
	}

	fRGBA(float r, float g, float b, float a = 1) :
		R(r),
		G(g),
		B(b),
		A(a)
	{
	}

	fRGBA(ARGB argb) :
#if SSE_VERSION >= 2
		mm(_mm_div_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128((unsigned int)argb), _mm_setzero_si128()), _mm_setzero_si128())), _mm_set1_ps(255.0f)))
#else
		R(argb.R / 255.0f),
		G(argb.G / 255.0f),
		B(argb.B / 255.0f),
		A(argb.A / 255.0f)
#endif
	{
#if SSE_VERSION >= 2
		mm = _mm_shuffle_ps(mm, mm, _MM_SHUFFLE(3,0,1,2));
#endif
	}

#if SSE_VERSION >= 2
	fRGBA(__m128 _mm) :
		mm(_mm)
	{
	}
#endif

	explicit operator ARGB() const
	{
#if SSE_VERSION >= 2
		return ARGB(_mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(_mm_cvttps_epi32(_mm_mul_ps(mm,_mm_set1_ps(255.0f))), _mm_setzero_si128()), _mm_setzero_si128())));
#else
		const fRGBA Temp = Saturate(*this) * 255;
		return ARGB((byte)Temp.R, (byte)Temp.G, (byte)Temp.B, (byte)Temp.A);
#endif
	}

	explicit operator RGB() const
	{
#if SSE_VERSION >= 2
		const ARGB Temp = (ARGB)*this;
		return RGB(Temp.R, Temp.G, Temp.B);
#else
		const fRGBA Temp = Saturate(*this) * 255;
		return RGB((byte)Temp.R, (byte)Temp.G, (byte)Temp.B);
#endif
	}

	fRGBA operator +(const fRGBA& rhs) const
	{
#if SSE_VERSION >= 2
		return fRGBA(_mm_add_ps(mm, rhs.mm));
#else
		return fRGBA(R + rhs.R, G + rhs.G, B + rhs.B, A + rhs.A);
#endif
	}

	fRGBA operator -(const fRGBA& rhs) const
	{
#if SSE_VERSION >= 2
		return fRGBA(_mm_sub_ps(mm, rhs.mm));
#else
		return fRGBA(R - rhs.R, G - rhs.G, B - rhs.B, A - rhs.A);
#endif
	}

	//template<typename type>
	//fARGB operator *(const type& rhs) const
	//{
	//	return fARGB((float)(R * rhs), (float)(G * rhs), (float)(B * rhs), (float)(A * rhs));
	//}

	//template<typename type>
	//const fARGB& operator *=(const type& rhs)
	//{
	//	R = (float)(R * rhs);
	//	G = (float)(G * rhs);
	//	B = (float)(B * rhs);
	//	A = (float)(A * rhs);
	//	return *this;
	//}

	fRGBA operator *(float rhs) const
	{
#if SSE_VERSION >= 2
		return fRGBA(_mm_mul_ps(mm, _mm_set1_ps(rhs)));
#else
		return fRGBA((float)(R * rhs), (float)(G * rhs), (float)(B * rhs), (float)(A * rhs));
#endif
	}

	const fRGBA& operator *=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_mul_ps(mm, _mm_set1_ps(rhs));
#else
		R = (float)(R * rhs);
		G = (float)(G * rhs);
		B = (float)(B * rhs);
		A = (float)(A * rhs);
#endif
		return *this;
	}

	//friend fARGB Lerp(const fARGB& X, const fARGB& Y, float Alpha)
	//{
	//	fARGB Result;
	//	Result.R = Lerp(X.R, Y.R, Alpha);
	//	Result.G = Lerp(X.G, Y.G, Alpha);
	//	Result.B = Lerp(X.B, Y.B, Alpha);
	//	Result.A = Lerp(X.A, Y.A, Alpha);
	//	return Result;
	//}

	friend fRGBA Saturate(const fRGBA& X)
	{
		fRGBA Result;
#if SSE_VERSION >= 2
		Result.mm = _mm_max_ps(_mm_min_ps(X.mm, _mm_set1_ps(1.0f)), _mm_setzero_ps());
#else
		Result.R = Saturate(X.R);
		Result.G = Saturate(X.G);
		Result.B = Saturate(X.B);
		Result.A = Saturate(X.A);
#endif
		return Result;
	}
};
