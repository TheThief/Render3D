#pragma once

#include "globaldefs.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <stdlib.h>

float forceinline frand()
{
	return (float)rand() / (RAND_MAX+1);
}

float forceinline frand(float min, float max)
{
	return frand() * (max-min) + min;
}

template<typename type>
type Lerp(const type& X, const type& Y, float Alpha)
{
	return (type)(X + (Y-X)*Alpha);
}

float forceinline Parametrize(float Value, float Min, float Max)
{
	return (Value-Min) / (Max-Min);
}

float forceinline Truncate(float X)
{
#if SSE_VERSION >= 4
	__m128 t = _mm_set_ss(X);
	return _mm_cvtss_f32(_mm_round_ss(t, t, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC));
#else
	if (X > 8388608)
	{
		// not enough precision to support fractions, so truncate is a no-op
		return X;
	}
	//return floor(X);
	return (float)(int)(X);
#endif
}

int forceinline TruncateToInt(float X)
{
	return (int)(X);
}

int forceinline RoundToInt(float X)
{
	return (int)(X + 0.5f);
}

float forceinline Fract(float X)
{
	//float id;
	//return modff(X, &id);
	return X - Truncate(X);
}

int forceinline Wrap(int X, int Min, int Max)
{
	int Wrapped = (X - Min) % (Max - Min);
	if (X < Min)
	{
		Wrapped += Max;
	}
	else
	{
		Wrapped += Min;
	}
	return Wrapped;
}

template<typename type>
type Average(const type& X, const type& Y)
{
	return X + (Y-X)/2;
}

template<typename type>
void Swap(type& X, type& Y)
{
	const type temp = X;
	X = Y;
	Y = temp;
}

template<typename type>
type Clamp(const type& X, const type& Min, const type& Max)
{
	if (X > Max)
	{
		return Max;
	}
	else if (X < Min)
	{
		return Min;
	}
	else
	{
		return X;
	}
}

template<typename type>
type Max(const type& X, const type& Y)
{
	if (Y > X)
	{
		return Y;
	}
	else
	{
		return X;
	}
}

template<typename type>
type Min(const type& X, const type& Y)
{
	if (Y < X)
	{
		return Y;
	}
	else
	{
		return X;
	}
}

template<typename type>
type Saturate(const type& X)
{
	return Clamp<type>(X, 0, 1);
}

// can't tell if this is faster...
//#if SSE_VERSION >= 2
//float Saturate(float X)
//{
//	// _mm_cvtss_f32() confuses the optimiser into using the x87 unit instead of SSE after a call to this function
//	//return _mm_cvtss_f32(_mm_max_ss(_mm_min_ss(_mm_set_ss(X), _mm_set_ss(1.0f)), _mm_setzero_ps()));
//	return _mm_max_ss(_mm_min_ss(_mm_set_ss(X), _mm_set_ss(1.0f)), _mm_setzero_ps()).m128_f32[0];
//}
//#endif

float forceinline Rcp(float X)
{
#if SSE_VERSION >= 2
	// _mm_cvtss_f32() confuses the VC++ optimiser into using the x87 unit instead of SSE after a call to this function
//#if __GNUC__
	return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(X)));
//#else
//	return _mm_rcp_ss(_mm_set_ss(X)).m128_f32[0];
//#endif
#else
	return 1/X;
#endif
}

#include<iterator>
template<typename T, T Step = 1>
class NumericIterator
{
	T Value;

public:
	typedef std::random_access_iterator_tag iterator_category;
	typedef T value_type;
	typedef T difference_type;
	//typedef T pointer;   // not sure what is correct here
	//typedef T reference; // ditto

	NumericIterator(T Value)
		: Value(Value)
	{
	}

	bool operator==(const NumericIterator<T, Step>& rhs) const
	{
		return Value == rhs.Value;
	}

	bool operator!=(const NumericIterator<T, Step>& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator<(const NumericIterator<T, Step>& rhs) const
	{
		return Value < rhs.Value;
	}

	bool operator>(const NumericIterator<T, Step>& rhs) const
	{
		return Value > rhs.Value;
	}

	bool operator<=(const NumericIterator<T, Step>& rhs) const
	{
		return Value <= rhs.Value;
	}

	bool operator>=(const NumericIterator<T, Step>& rhs) const
	{
		return Value >= rhs.Value;
	}

	value_type operator*() const
	{
		return Value;
	};

	// operator->() intentionally undefined as pointer typedef is undefined

	NumericIterator<T, Step>& operator++()
	{
		Value += Step;
		return *this;
	};

	NumericIterator<T, Step> operator++(int)
	{
		NumericIterator<T, Step> Result = *this;
		++(*this);
		return Result;
	};

	NumericIterator<T, Step>& operator--()
	{
		Value -= Step;
		return *this;
	};

	NumericIterator<T, Step> operator--(int)
	{
		NumericIterator<T, Step> Result = *this;
		--(*this);
		return Result;	};

	NumericIterator<T, Step>& operator+=(difference_type inc)
	{
		Value += inc * Step;
		return *this;
	}

	NumericIterator<T, Step>& operator-=(difference_type dec)
	{
		Value -= dec * Step;
		return *this;
	}

	NumericIterator<T, Step> operator+(difference_type inc)
	{
		return NumericIterator<T, Step>(Value + inc * Step);
	}

	NumericIterator<T, Step> operator-(difference_type dec) const
	{
		return NumericIterator<T, Step>(Value - dec * Step);
	}

	difference_type operator-(const NumericIterator<T>& rhs) const
	{
		return (Value - rhs.Value) / Step;
	}

	value_type operator[](difference_type offset) const
	{
		return Value + offset * Step;
	}
};

template<typename T, T Step = 1>
class NumericRange
{
	T Start;
	T End;

public:
	NumericRange(T Start, T End)
		: Start(Start), End(End)
	{
	}

	NumericIterator<T, Step> begin() const
	{
		return NumericIterator<T, Step>(Start);
	}

	NumericIterator<T, Step> end() const
	{
		return NumericIterator<T, Step>(End);
	}

	//NumericIterator<T, -Step> rbegin() const
	//{
	//	return NumericIterator<T, -Step>(End - 1);
	//}

	//NumericIterator<T, -Step> rend() const
	//{
	//	return NumericIterator<T, -Step>(Start - 1);
	//}
};
