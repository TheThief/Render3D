#pragma once

#include "vector_math.h"
#include "quatmath.h"

struct Matrix
{
	Vector4 v[4];

	Matrix()
	{
		v[0] = Vector4(1,0,0,0);
		v[1] = Vector4(0,1,0,0);
		v[2] = Vector4(0,0,1,0);
		v[3] = Vector4(0,0,0,1);
	}

	Matrix operator *(const Matrix& rhs) const
	{
		Matrix Result;
#if SSE_VERSION >= 2
		// First row of result (Matrix1[0] * Matrix2).
		Result.v[0].mm = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[0].mm, v[0].mm, _MM_SHUFFLE(0, 0, 0, 0)), rhs.v[0].mm ),
		                 _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[0].mm, v[0].mm, _MM_SHUFFLE(1, 1, 1, 1)), rhs.v[1].mm ),
		                 _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[0].mm, v[0].mm, _MM_SHUFFLE(2, 2, 2, 2)), rhs.v[2].mm ),
		                             _mm_mul_ps( _mm_shuffle_ps(v[0].mm, v[0].mm, _MM_SHUFFLE(3, 3, 3, 3)), rhs.v[3].mm ) ) ) );

		// Second row of result (Matrix1[1] * Matrix2).
		Result.v[1].mm = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[1].mm, v[1].mm, _MM_SHUFFLE(0, 0, 0, 0)), rhs.v[0].mm ),
		                 _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[1].mm, v[1].mm, _MM_SHUFFLE(1, 1, 1, 1)), rhs.v[1].mm ),
		                 _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[1].mm, v[1].mm, _MM_SHUFFLE(2, 2, 2, 2)), rhs.v[2].mm ),
		                             _mm_mul_ps( _mm_shuffle_ps(v[1].mm, v[1].mm, _MM_SHUFFLE(3, 3, 3, 3)), rhs.v[3].mm ) ) ) );

		// Third row of result (Matrix1[2] * Matrix2).
		Result.v[2].mm = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[2].mm, v[2].mm, _MM_SHUFFLE(0, 0, 0, 0)), rhs.v[0].mm ),
		                 _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[2].mm, v[2].mm, _MM_SHUFFLE(1, 1, 1, 1)), rhs.v[1].mm ),
		                 _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[2].mm, v[2].mm, _MM_SHUFFLE(2, 2, 2, 2)), rhs.v[2].mm ),
		                             _mm_mul_ps( _mm_shuffle_ps(v[2].mm, v[2].mm, _MM_SHUFFLE(3, 3, 3, 3)), rhs.v[3].mm ) ) ) );

		// Fourth row of result (Matrix1[3] * Matrix2).
		Result.v[3].mm = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[3].mm, v[3].mm, _MM_SHUFFLE(0, 0, 0, 0)), rhs.v[0].mm ),
		                 _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[3].mm, v[3].mm, _MM_SHUFFLE(1, 1, 1, 1)), rhs.v[1].mm ),
		                 _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(v[3].mm, v[3].mm, _MM_SHUFFLE(2, 2, 2, 2)), rhs.v[2].mm ),
		                             _mm_mul_ps( _mm_shuffle_ps(v[3].mm, v[3].mm, _MM_SHUFFLE(3, 3, 3, 3)), rhs.v[3].mm ) ) ) );
#else
		// First row of result (Matrix1[0] * Matrix2).
		Result.v[0] = Vector4(v[0].x, v[0].x, v[0].x, v[0].x) * rhs.v[0]
		            + Vector4(v[0].y, v[0].y, v[0].y, v[0].y) * rhs.v[1]
		            + Vector4(v[0].z, v[0].z, v[0].z, v[0].z) * rhs.v[2]
		            + Vector4(v[0].w, v[0].w, v[0].w, v[0].w) * rhs.v[3];

		// Second row of result (Matrix1[1] * Matrix2).
		Result.v[1] = Vector4(v[1].x, v[1].x, v[1].x, v[1].x) * rhs.v[0]
		            + Vector4(v[1].y, v[1].y, v[1].y, v[1].y) * rhs.v[1]
		            + Vector4(v[1].z, v[1].z, v[1].z, v[1].z) * rhs.v[2]
		            + Vector4(v[1].w, v[1].w, v[1].w, v[1].w) * rhs.v[3];

		// Third row of result (Matrix1[2] * Matrix2).
		Result.v[2] = Vector4(v[2].x, v[2].x, v[2].x, v[2].x) * rhs.v[0]
		            + Vector4(v[2].y, v[2].y, v[2].y, v[2].y) * rhs.v[1]
		            + Vector4(v[2].z, v[2].z, v[2].z, v[2].z) * rhs.v[2]
		            + Vector4(v[2].w, v[2].w, v[2].w, v[2].w) * rhs.v[3];

		// Fourth row of result (Matrix1[3] * Matrix2).
		Result.v[3] = Vector4(v[3].x, v[3].x, v[3].x, v[3].x) * rhs.v[0]
		            + Vector4(v[3].y, v[3].y, v[3].y, v[3].y) * rhs.v[1]
		            + Vector4(v[3].z, v[3].z, v[3].z, v[3].z) * rhs.v[2]
		            + Vector4(v[3].w, v[3].w, v[3].w, v[3].w) * rhs.v[3];
#endif
		return Result;
	}

	Matrix& operator *=(const Matrix& rhs)
	{
		*this = *this * rhs;
		return *this;
	}

	friend Vector4 operator *(const Vector4& lhs, const Matrix& rhs)
	{
#if SSE_VERSION >= 2
		return Vector4( _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(lhs.mm, lhs.mm, _MM_SHUFFLE(0, 0, 0, 0)), rhs.v[0].mm ),
		                _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(lhs.mm, lhs.mm, _MM_SHUFFLE(1, 1, 1, 1)), rhs.v[1].mm ),
		                _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(lhs.mm, lhs.mm, _MM_SHUFFLE(2, 2, 2, 2)), rhs.v[2].mm ),
		                            _mm_mul_ps( _mm_shuffle_ps(lhs.mm, lhs.mm, _MM_SHUFFLE(3, 3, 3, 3)), rhs.v[3].mm ) ) ) ) );
#else
		return Vector4(lhs.x, lhs.x, lhs.x, lhs.x) * rhs.v[0]
		     + Vector4(lhs.y, lhs.y, lhs.y, lhs.y) * rhs.v[1]
		     + Vector4(lhs.z, lhs.z, lhs.z, lhs.z) * rhs.v[2]
		     + Vector4(lhs.w, lhs.w, lhs.w, lhs.w) * rhs.v[3];
#endif
	}

	friend Vector4& operator *=(Vector4& lhs, const Matrix& rhs)
	{
#if SSE_VERSION >= 2
		lhs = Vector4( _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(lhs.mm, lhs.mm, _MM_SHUFFLE(0, 0, 0, 0)), rhs.v[0].mm ),
		               _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(lhs.mm, lhs.mm, _MM_SHUFFLE(1, 1, 1, 1)), rhs.v[1].mm ),
		               _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(lhs.mm, lhs.mm, _MM_SHUFFLE(2, 2, 2, 2)), rhs.v[2].mm ),
		                           _mm_mul_ps( _mm_shuffle_ps(lhs.mm, lhs.mm, _MM_SHUFFLE(3, 3, 3, 3)), rhs.v[3].mm ) ) ) ) );
#else
		lhs = Vector4(lhs.x, lhs.x, lhs.x, lhs.x) * rhs.v[0]
		    + Vector4(lhs.y, lhs.y, lhs.y, lhs.y) * rhs.v[1]
		    + Vector4(lhs.z, lhs.z, lhs.z, lhs.z) * rhs.v[2]
		    + Vector4(lhs.w, lhs.w, lhs.w, lhs.w) * rhs.v[3];
#endif
		return lhs;
	}

	static Matrix ConstructTranslation(const Vector3& translation)
	{
		Matrix Result;

		Result.v[0] = Vector4(1, 0, 0, 0);
		Result.v[1] = Vector4(0, 1, 0, 0);
		Result.v[2] = Vector4(0, 0, 1, 0);
		Result.v[3] = Vector4(translation, 1);
		return Result;
	}

	static Matrix ConstructScale(const Vector3& scale)
	{
		Matrix Result;

		Result.v[0] = Vector4(scale.x, 0, 0, 0);
		Result.v[1] = Vector4(0, scale.y, 0, 0);
		Result.v[2] = Vector4(0, 0, scale.z, 0);
		Result.v[3] = Vector4(0, 0, 0, 1);
		return Result;
	}

	static Matrix ConstructXRotation(float rotation) // pitch
	{
		Matrix Result;

		const float SinX = sin(rotation);
		const float CosX = cos(rotation);

		Result.v[0] = Vector4(    1,    0,    0,   0);
		Result.v[1] = Vector4(    0, CosX,-SinX,   0);
		Result.v[2] = Vector4(    0, SinX, CosX,   0);
		Result.v[3] = Vector4(    0,    0,    0,   1);
		return Result;
	}

	static Matrix ConstructYRotation(float rotation) // yaw
	{
		Matrix Result;

		const float SinY = sin(rotation);
		const float CosY = cos(rotation);

		Result.v[0] = Vector4( CosY,    0, SinY,   0);
		Result.v[1] = Vector4(    0,    1,    0,   0);
		Result.v[2] = Vector4(-SinY,    0, CosY,   0);
		Result.v[3] = Vector4(    0,    0,    0,   1);
		return Result;
	}

	static Matrix ConstructZRotation(float rotation) // roll
	{
		Matrix Result;

		const float SinZ = sin(rotation);
		const float CosZ = cos(rotation);

		Result.v[0] = Vector4( CosZ, SinZ,    0,   0);
		Result.v[1] = Vector4(-SinZ, CosZ,    0,   0);
		Result.v[2] = Vector4(    0,    0,    1,   0);
		Result.v[3] = Vector4(    0,    0,    0,   1);
		return Result;
	}

	static Matrix ConstructPerspective_LH()
	{
		Matrix Result;

		const float ZNear = 0.1f;
		const float ZFar = 1000.0f;

		Result.v[0] = Vector4(    1,    0,    0,   0);
		Result.v[1] = Vector4(    0,    1,    0,   0);
		Result.v[2] = Vector4(    0,    0,    ZFar / (ZFar - ZNear),   1);
		Result.v[3] = Vector4(    0,    0,    -ZNear * ZFar / (ZFar - ZNear),   0);
		return Result;
	}

	static Matrix ConstructPerspective_RH()
	{
		Matrix Result;

		const float ZNear = 0.1f;
		const float ZFar = 1000.0f;

		Result.v[0] = Vector4(    1,    0,    0,   0);
		Result.v[1] = Vector4(    0,    1,    0,   0);
		Result.v[2] = Vector4(    0,    0,    -ZFar / (ZFar - ZNear),   -1);
		Result.v[3] = Vector4(    0,    0,    ZNear * -ZFar / (ZFar - ZNear),   0);
		return Result;
	}

	static Matrix ConstructPerspective_Infinite_LH()
	{
		Matrix Result;

		const float ZNear = 0.1f;

		Result.v[0] = Vector4(1, 0, 0, 0);
		Result.v[1] = Vector4(0, 1, 0, 0);
		Result.v[2] = Vector4(0, 0, 1, 1);
		Result.v[3] = Vector4(0, 0, -ZNear, 0);
		return Result;
	}

	static Matrix ConstructPerspective_Reversed_LH()
	{
		Matrix Result;

		const float ZNear = 0.1f;
		const float ZFar = 1000.0f;

		Result.v[0] = Vector4(1, 0, 0, 0);
		Result.v[1] = Vector4(0, 1, 0, 0);
		Result.v[2] = Vector4(0, 0, 1 - ZFar / (ZFar - ZNear), 1);
		Result.v[3] = Vector4(0, 0, ZNear * ZFar / (ZFar - ZNear), 0);
		return Result;
	}

	static Matrix ConstructPerspective_Reversed_Infinite_LH()
	{
		Matrix Result;

		const float ZNear = 0.1f;

		Result.v[0] = Vector4(1, 0, 0, 0);
		Result.v[1] = Vector4(0, 1, 0, 0);
		Result.v[2] = Vector4(0, 0, 0, 1);
		Result.v[3] = Vector4(0, 0, ZNear, 0);
		return Result;
	}

	static Matrix ConstructFromQuaternion(const Quaternion& _q, const Vector3& _v = Vector3::zero)
	{
		Matrix Result;
		Result.v[0] = Vector4(_q.w*_q.w + _q.x*_q.x - _q.y*_q.y - _q.z*_q.z, 2*(_q.x*_q.y + _q.z*_q.w), 2*(_q.x*_q.z - _q.y*_q.w), 0);
		Result.v[1] = Vector4(2*(_q.x*_q.y - _q.z*_q.w), _q.w*_q.w - _q.x*_q.x + _q.y*_q.y - _q.z*_q.z, 2*(_q.y*_q.z + _q.x*_q.w), 0);
		Result.v[2] = Vector4(2*(_q.x*_q.z + _q.y*_q.w), 2*(_q.y*_q.z - _q.x*_q.w), _q.w*_q.w - _q.x*_q.x - _q.y*_q.y + _q.z*_q.z, 0);
		Result.v[3] = Vector4(_v, 1);
		return Result;
	}
};
