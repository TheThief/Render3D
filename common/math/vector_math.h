#pragma once

#include "globaldefs.h"
#include "scalarmath.h"

struct alignstruct(16) Vector2
{
	union
	{
		struct
		{
			float x,y;
		};
#if SSE_VERSION >= 2
		__m128 mm;
#endif
	};

	Vector2() = default;

	Vector2(float _x, float _y) :
		x(_x), y(_y)
	{
	}

#if SSE_VERSION >= 2
	explicit Vector2(__m128 _mm) :
		mm(_mm)
	{
	}
#endif

	bool operator ==(const Vector2& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}

	Vector2 operator +(const Vector2& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector2(_mm_add_ps(mm, rhs.mm));
#else
		return Vector2(x+rhs.x, y+rhs.y);
#endif
	}

	Vector2 operator -(const Vector2& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector2(_mm_sub_ps(mm, rhs.mm));
#else
		return Vector2(x-rhs.x, y-rhs.y);
#endif
	}

	const Vector2& operator +=(const Vector2& rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_add_ps(mm, rhs.mm);
#else
		x += rhs.x;
		y += rhs.y;
#endif
		return *this;
	}

	const Vector2& operator -=(const Vector2& rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_sub_ps(mm, rhs.mm);
#else
		x -= rhs.x;
		y -= rhs.y;
#endif
		return *this;
	}

	Vector2 operator *(const Vector2& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector2(_mm_mul_ps(mm, rhs.mm));
#else
		return Vector2(x*rhs.x, y*rhs.y);
#endif
	}

	Vector2 operator /(const Vector2& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector2(_mm_div_ps(mm, rhs.mm));
#else
		return Vector2(x/rhs.x, y/rhs.y);
#endif
	}

	Vector2 operator *(float rhs) const
	{
#if SSE_VERSION >= 2
		return Vector2(_mm_mul_ps(mm, _mm_load1_ps(&rhs)));
#else
		return Vector2(x*rhs, y*rhs);
#endif
	}

	Vector2 operator /(float rhs) const
	{
#if SSE_VERSION >= 2
		return Vector2(_mm_div_ps(mm, _mm_load1_ps(&rhs)));
#else
		return Vector2(x/rhs, y/rhs);
#endif
	}

	const Vector2& operator *=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_mul_ps(mm, _mm_load1_ps(&rhs));
#else
		x*=rhs;
		y*=rhs;
#endif
		return *this;
	}

	const Vector2& operator /=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_div_ps(mm, _mm_load1_ps(&rhs));
#else
		x /= rhs;
		y /= rhs;
#endif
		return *this;
	}

#if SSE_VERSION >= 2
	__m128 RcpSize() const
	{
		__m128 squared = _mm_mul_ps(mm, mm);
		__m128 temp = _mm_add_ps(squared, _mm_shuffle_ps(squared, squared, _MM_SHUFFLE(3, 2, 0, 1)));
		return _mm_rsqrt_ps(temp);
	}
#endif

	Vector2 Rcp() const
	{
#if SSE_VERSION >= 2
		return Vector2(_mm_rcp_ps(mm));
#else
		return Vector2(1/x, 1/y);
#endif
	}

	float SizeSquared() const
	{
		Vector2 squared = (*this) * (*this);
		return squared.x + squared.y;
	}

	float Size() const
	{
		Vector2 squared = (*this) * (*this);
		return sqrt(squared.x + squared.y);
	}

	Vector2 Normalized() const
	{
#if SSE_VERSION >= 2
		return Vector2(_mm_mul_ps(mm, RcpSize()));
#else
		return *this / Size();
#endif
	}
};

struct alignstruct(16) Vector3
{
	union
	{
		struct
		{
			float x, y, z;
		};
#if SSE_VERSION >= 2
		__m128 mm;
#endif
	};

	Vector3() = default;

	Vector3(float _f) :
		x(_f), y(_f), z(_f)
	{
	}

	Vector3(float _x, float _y, float _z) :
		x(_x), y(_y), z(_z)
	{
	}

	Vector3(const Vector2& _v, float _z = 0.0f) :
#if SSE_VERSION >= 2
		mm(_v.mm)
	{
		z = _z;
	}
#else
		x(_v.x), y(_v.y), z(_z)
	{
	}
#endif

#if SSE_VERSION >= 2
	explicit Vector3(__m128 _mm) :
		mm(_mm)
	{
	}
#endif

	bool operator==(const Vector3& rhs)
	{
//#if SSE_VERSION >= 2
		//return _mm_movemask_epi8(_mm_castps_si128(_mm_cmpeq_ps(mm, rhs.mm))); // todo
//#else
		return x == rhs.x && y == rhs.y && z == rhs.z;
//#endif
	}

	static const Vector3 signmask;

	Vector3 operator -() const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_xor_ps(signmask.mm, mm));
#else
		return Vector3(-x, -y, -z);
#endif
	}

	Vector3 operator +(const Vector3& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_add_ps(mm, rhs.mm));
#else
		return Vector3(x+rhs.x, y+rhs.y, z+rhs.z);
#endif
	}

	Vector3 operator -(const Vector3& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_sub_ps(mm, rhs.mm));
#else
		return Vector3(x-rhs.x, y-rhs.y, z-rhs.z);
#endif
	}

	const Vector3& operator +=(const Vector3& rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_add_ps(mm, rhs.mm);
#else
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
#endif
		return *this;
	}

	const Vector3& operator -=(const Vector3& rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_sub_ps(mm, rhs.mm);
#else
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
#endif
		return *this;
	}

	Vector3 operator *(const Vector3& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_mul_ps(mm, rhs.mm));
#else
		return Vector3(x*rhs.x, y*rhs.y, z*rhs.z);
#endif
	}

	Vector3 operator /(const Vector3& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_div_ps(mm, rhs.mm));
#else
		return Vector3(x/rhs.x, y/rhs.y, z/rhs.z);
#endif
	}

	Vector3 operator *(float rhs) const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_mul_ps(mm, _mm_load1_ps(&rhs)));
#else
		return Vector3(x*rhs, y*rhs, z*rhs);
#endif
	}

	Vector3 operator /(float rhs) const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_div_ps(mm, _mm_load1_ps(&rhs)));
#else
		return Vector3(x/rhs, y/rhs, z/rhs);
#endif
	}

	const Vector3& operator *=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_mul_ps(mm, _mm_load1_ps(&rhs));
#else
		x *= rhs;
		y *= rhs;
		z *= rhs;
#endif
		return *this;
	}

	const Vector3& operator /=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_div_ps(mm, _mm_load1_ps(&rhs));
#else
		x /= rhs;
		y /= rhs;
		z /= rhs;
#endif
		return *this;
	}

	Vector3 Rcp() const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_rcp_ps(mm));
#else
		return Vector3(1/x, 1/y, 1/z);
#endif
	}

#if SSE_VERSION >= 2
	__m128 RcpSize() const
	{
		__m128 squared = _mm_mul_ps(mm, mm);
		__m128 temp = _mm_add_ps(squared, _mm_shuffle_ps(squared, squared, _MM_SHUFFLE(3, 0, 2, 1)));
		temp = _mm_add_ps(temp, _mm_shuffle_ps(squared, squared, _MM_SHUFFLE(3, 1, 0, 2)));
		return _mm_rsqrt_ps(temp);
	}
#endif

	float SizeSquared() const
	{
		return this->Dot(*this);
	}

	float Size() const
	{
		return sqrt(SizeSquared());
	}

	Vector3 Normalized() const
	{
#if SSE_VERSION >= 2
		return Vector3(_mm_mul_ps(mm, RcpSize()));
#else
		return *this / Size();
#endif
	}

	float Dot(const Vector3& rhs) const
	{
#if SSE_VERSION >= 4
		return _mm_cvtss_f32(_mm_dp_ps(mm, rhs.mm, 0x7f));
#else
		Vector3 temp = (*this) * rhs;
		return temp.x + temp.y + temp.z;
#endif
	}

	Vector3 Cross(const Vector3& rhs) const
	{
#if SSE_VERSION >= 2
		return
			Vector3(_mm_sub_ps(
				_mm_mul_ps(_mm_shuffle_ps(mm, mm, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(rhs.mm, rhs.mm, _MM_SHUFFLE(3, 1, 0, 2))),
				_mm_mul_ps(_mm_shuffle_ps(mm, mm, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(rhs.mm, rhs.mm, _MM_SHUFFLE(3, 0, 2, 1)))
			));
#else
		return
			Vector3
			{
				y * rhs.z - z * rhs.y,
				z * rhs.x - x * rhs.z,
				x * rhs.y - y * rhs.x
			};
#endif
	}

	const Vector2& xy() const
	{
		return *(Vector2*)this;
	}

	static const Vector3 zero;
	static const Vector3 xaxis;
	static const Vector3 yaxis;
	static const Vector3 zaxis;
};

struct alignstruct(16) Vector4
{
	union
	{
		struct
		{
			float x,y,z,w;
		};
#if SSE_VERSION >= 2
		__m128 mm;
#endif
	};

	Vector4() = default;

	Vector4(float _x, float _y, float _z, float _w=1.0f) :
		x(_x), y(_y), z(_z), w(_w)
	{
	}

	Vector4(const Vector3& _v, float _w=1.0f) :
#if SSE_VERSION >= 2
		mm(_v.mm)
	{
		w = _w;
	}
#else
		x(_v.x), y(_v.y), z(_v.z), w(_w)
	{
	}
#endif

#if SSE_VERSION >= 2
	explicit Vector4(__m128 _mm) :
		mm(_mm)
	{
	}
#endif

//	Vector4& operator=(const Vector4& rhs)
//	{
//		// TODO: assignment isn't actually defined for __m128
//#if SSE_VERSION >= 2
//		mm = _mm_load_ps(&rhs.x);
//		//mm = rhs.mm;
//#else
//		x = rhs.x;
//		y = rhs.y;
//		z = rhs.z;
//		w = rhs.w;
//#endif
//		return *this;
//	}

	operator Vector3() const
	{
		if (w == 0.0f || w == 1.0f)
		{
			return (Vector3&)*this;
		}
		else
		{
			return ((Vector3&)*this) / w;
		}
	}

	Vector4 operator +(const Vector4& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector4(_mm_add_ps(mm, rhs.mm));
#else
		return Vector4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w);
#endif
	}

	Vector4 operator -(const Vector4& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector4(_mm_sub_ps(mm, rhs.mm));
#else
		return Vector4(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w);
#endif
	}

	Vector4 operator *(const Vector4& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector4(_mm_mul_ps(mm, rhs.mm));
#else
		return Vector4(x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w);
#endif
	}

	Vector4 operator /(const Vector4& rhs) const
	{
#if SSE_VERSION >= 2
		return Vector4(_mm_div_ps(mm, rhs.mm));
#else
		return Vector4(x/rhs.x, y/rhs.y, z/rhs.z, w*rhs.w);
#endif
	}

	Vector4 operator *(float rhs) const
	{
#if SSE_VERSION >= 2
		return Vector4(_mm_mul_ps(mm, _mm_set1_ps(rhs)));
#else
		return Vector4(x*rhs, y*rhs, z*rhs, w*rhs);
#endif
	}

	Vector4 operator /(float rhs) const
	{
#if SSE_VERSION >= 2
		return Vector4(_mm_div_ps(mm, _mm_set1_ps(rhs)));
#else
		return Vector4(x/rhs, y/rhs, z/rhs, w/rhs);
#endif
	}

	const Vector4& operator *=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_mul_ps(mm, _mm_set1_ps(rhs));
#else
		x*=rhs;
		y*=rhs;
		z*=rhs;
		w*=rhs;
#endif
		return *this;
	}

	const Vector4& operator /=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_div_ps(mm, _mm_set1_ps(rhs));
#else
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
#endif
		return *this;
	}

	Vector4 Rcp() const
	{
#if SSE_VERSION >= 2
		return Vector4(_mm_rcp_ps(mm));
#else
		return Vector4(1/x, 1/y, 1/z, 1/w);
#endif
	}

#if SSE_VERSION >= 2
	__m128 RcpSize() const
	{
		__m128 squared = _mm_mul_ps(mm, mm);
		__m128 temp = _mm_add_ps(squared, _mm_shuffle_ps(squared, squared, _MM_SHUFFLE(3, 0, 2, 1)));
		temp = _mm_add_ps(temp, _mm_shuffle_ps(squared, squared, _MM_SHUFFLE(3, 1, 0, 2)));
		return _mm_rsqrt_ps(temp);
	}
#endif

	float SizeSquared() const
	{
		Vector4 squared = (*this) * (*this);
		return squared.x + squared.y + squared.z + squared.w;
	}

	float Size() const
	{
		return sqrt(SizeSquared());
	}

	Vector4 Normalized() const
	{
#if SSE_VERSION >= 2
		return Vector4(_mm_mul_ps(mm, RcpSize()));
#else
		return *this / Size();
#endif
	}

	const Vector2& xy() const
	{
		return *(Vector2*)this;
	}

	const Vector3& xyz() const
	{
		return *(Vector3*)this;
	}
};

enum ClipResult
{
	CR_EntirelyInside,
	CR_EntirelyOutside,
	CR_Leaving,
	CR_Entering,
};

inline float PointToPlaneSpace(const Vector4& Point, const Vector4& Plane)
{
	return Point.xyz().Dot(Plane.xyz()) - Plane.w;
}

ClipResult __inline ClipLineAgainstPlane(float Start, float End, float& Out)
{
	if (Start >= 0 && End >= 0)
	{
		return CR_EntirelyInside;
	}
	else if (Start <= 0 && End <= 0)
	{
		return CR_EntirelyOutside;
	}
	else if (Start >= 0 && End <= 0)
	{
		Out = Start / (Start - End);
		return CR_Leaving;
	}
	else //if (A <= 0 && B >= 0)
	{
		Out = Start / (Start - End);
		return CR_Entering;
	}
}

ClipResult __inline ClipLineAgainstPlane(const Vector4& Start, const Vector4& End, float& Out, const Vector4& Plane)
{
	return ClipLineAgainstPlane(PointToPlaneSpace(Start, Plane), PointToPlaneSpace(End, Plane), Out);
}

int __inline ClipTriangleAgainstPlane(Vector4 In[3], Vector4 (&Out)[4], const Vector4& Plane)
{
	float dot[3] = { PointToPlaneSpace(In[0], Plane), PointToPlaneSpace(In[1], Plane), PointToPlaneSpace(In[2], Plane) };

	int Verts = 0;
	int PrevInVert = 2;
	for (int InVert = 0; InVert < 3; PrevInVert = InVert, InVert++)
	{
		float frac;
		switch(ClipLineAgainstPlane(dot[PrevInVert], dot[InVert], frac))
		{
		case CR_EntirelyInside:
			Out[Verts] = In[InVert];
			Verts++;
			break;
		case CR_EntirelyOutside:
			break;
		case CR_Leaving:
			Out[Verts] = Lerp(In[PrevInVert], In[InVert], frac);
			Verts++;
			break;
		case CR_Entering:
			Out[Verts] = Lerp(In[PrevInVert], In[InVert], frac);
			Verts++;
			Out[Verts] = In[InVert];
			Verts++;
			break;
		}
	}
	return Verts;
}
