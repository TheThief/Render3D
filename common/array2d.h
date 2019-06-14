#pragma once

#include "globaldefs.h"
#include "math/scalarmath.h"
#include <vector>

template<typename type, typename underlying_type = std::vector<type>>
class Array2d
{
	underlying_type underlying_vector;
	unsigned int mWidth;
	unsigned int mHeight;

public:
	Array2d() :
		underlying_vector(),
		mWidth(0),
		mHeight(0)
	{
	}

	Array2d(unsigned int _width, unsigned int _height) :
		mWidth(_width),
		mHeight(_height)
	{
		const auto size = mWidth * mHeight;
		underlying_vector.resize(size);
	}

	~Array2d() = default;

	Array2d(const Array2d& rhs) = default;
	Array2d& operator =(const Array2d& rhs) = default;

	Array2d(Array2d&& rhs) = default;
	Array2d& operator =(Array2d&& rhs) = default;

	type& Get(unsigned int x, unsigned int y)
	{
		assert(x < mWidth);
		assert(y < mHeight);
		return underlying_vector[y*mWidth + x];
	}

	const type& Get(unsigned int x, unsigned int y) const
	{
		assert(x < mWidth);
		assert(y < mHeight);
		return underlying_vector[y*mWidth + x];
	}

	type& operator() (unsigned int x, unsigned int y)
	{
		assert(x < mWidth);
		assert(y < mHeight);
		return underlying_vector[y*mWidth + x];
	}

	const type& operator() (unsigned int x, unsigned int y) const
	{
		assert(x < mWidth);
		assert(y < mHeight);
		return underlying_vector[y*mWidth + x];
	}

	unsigned int GetWidth() const
	{
		return mWidth;
	}

	unsigned int GetHeight() const
	{
		return mHeight;
	}

	type* GetScanlineDataPointer(unsigned int y)
	{
		return &underlying_vector[y*mWidth];
	}

	type* GetDataPointer()
	{
		return underlying_vector.data();
	}

	unsigned int GetPitch() const
	{
		return mWidth*sizeof(type);
	}

	template<typename interp> // =type
	forceinline interp Sample(float x, float y) const
	{
		int ix = TruncateToInt(x);
		int iy = TruncateToInt(y);
		int ix2 = ix + 1;
		int iy2 = iy + 1;
		if (ix < 0) { ix = ix2 = 0; }
		else if (ix2 >= (int)mWidth)  { ix = ix2 = mWidth - 1; }
		if (iy < 0) { iy = iy2 = 0; }
		else if (iy2 >= (int)mHeight) { iy = iy2 = mHeight - 1; }
		const float fx = Fract(x);
		const float fy = Fract(y);
		return Lerp(
			Lerp( (interp)Get(ix, iy), (interp)Get(ix2, iy), fx ),
			Lerp( (interp)Get(ix, iy2), (interp)Get(ix2, iy2), fx ),
			fy );
	}

	template<typename interp> // =type
	forceinline interp SampleWrap(float x, float y) const
	{
		const int ix = Wrap(TruncateToInt(x), 0, mWidth);
		const int iy = Wrap(TruncateToInt(y), 0, mHeight);
		const int ix2 = Wrap(ix + 1, 0, mWidth);
		const int iy2 = Wrap(iy + 1, 0, mHeight);
		const float fx = Fract(x);
		const float fy = Fract(y);
		return Lerp(
			Lerp( (interp)Get(ix, iy), (interp)Get(ix2, iy), fx ),
			Lerp( (interp)Get(ix, iy2), (interp)Get(ix2, iy2), fx ),
			fy );
	}
};
