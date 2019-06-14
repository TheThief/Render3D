#pragma once

#include "globaldefs.h"
#include "scalarmath.h"
#include "vector_math.h"

struct alignstruct(16) Quaternion
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

	Quaternion() = default;

	Quaternion(float _x, float _y, float _z, float _w) :
		x(_x), y(_y), z(_z), w(_w)
	{
	}

	Quaternion(const Vector3& _axis) :
#if SSE_VERSION >= 2
		mm(_axis.mm)
	{
		w = 0;
	}
#else
		x(_axis.x), y(_axis.y), z(_axis.z), w(0)
	{
	}
#endif

	Quaternion(const Vector3& _axis, float _angle) :
#if SSE_VERSION >= 2
		mm((_axis * sinf(_angle/2)).mm)
	{
		w = cosf(_angle/2);
	}
#else
		x(_axis.x * sinf(_angle/2)), y(_axis.y * sinf(_angle/2)), z(_axis.z * sinf(_angle/2)), w(cosf(_angle/2))
	{
	}
#endif

#if SSE_VERSION >= 2
	explicit Quaternion(__m128 _mm) :
		mm(_mm)
	{
	}
#endif

	Quaternion operator *(float rhs) const
	{
#if SSE_VERSION >= 2
		return Quaternion(_mm_mul_ps(mm, _mm_load1_ps(&rhs)));
#else
		return Quaternion(x*rhs, y*rhs, z*rhs, w*rhs);
#endif
	}

	Quaternion operator /(float rhs) const
	{
#if SSE_VERSION >= 2
		return Quaternion(_mm_div_ps(mm, _mm_load1_ps(&rhs)));
#else
		return Quaternion(x/rhs, y/rhs, z/rhs, w/rhs);
#endif
	}

	const Quaternion& operator *=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_mul_ps(mm, _mm_load1_ps(&rhs));
#else
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;
#endif
		return *this;
	}

	const Quaternion& operator /=(float rhs)
	{
#if SSE_VERSION >= 2
		mm = _mm_div_ps(mm, _mm_load1_ps(&rhs));
#else
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
#endif
		return *this;
	}

	Quaternion operator *(const Quaternion& rhs) const
	{
		return Quaternion(
			(w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y),
			(w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x),
			(w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w),
			(w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z)
			);
	}

	friend Vector3 operator *(const Vector3& lhs, const Quaternion& rhs)
	{
#if SSE_VERSION >= 2
		return Vector3((rhs * Quaternion(lhs) * rhs.Conjugate()).mm);
#else
		Quaternion result = rhs * Quaternion(lhs) * rhs.Conjugate();
		return Vector3(result.x, result.y, result.z);
#endif
	}

	static const Quaternion conjmask;

	Quaternion Conjugate() const
	{
#if SSE_VERSION >= 2
		return Quaternion(_mm_xor_ps(mm, conjmask.mm));
#else
		return Quaternion(-x, -y, -z, w);
#endif
	}

//	Vector4 Rcp() const
//	{
//#if SSE_VERSION >= 2
//		return Vector4(_mm_rcp_ps(mm));
//#else
//		return Vector4(1/x, 1/y, 1/z, 1/w);
//#endif
//	}

#if SSE_VERSION >= 2
	__m128 RcpSize() const
	{
		__m128 squared = _mm_mul_ps(mm, mm);
		__m128 temp = _mm_add_ps(squared, _mm_shuffle_ps(squared, squared, _MM_SHUFFLE(3, 0, 2, 1)));
		temp = _mm_add_ps(temp, _mm_shuffle_ps(squared, squared, _MM_SHUFFLE(3, 1, 0, 2)));
		return _mm_rsqrt_ps(temp);
	}
#endif

	float Size() const
	{
		return sqrt(x*x + y*y + z*z + w*w);
	}

	Quaternion Normalized() const
	{
#if SSE_VERSION >= 2
		return Quaternion(_mm_mul_ps(mm, RcpSize()));
#else
		float _size = Size();
		return Quaternion(x/_size, y/_size, z/_size, w/_size);
#endif
	}
};
