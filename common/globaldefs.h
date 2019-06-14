#pragma once

#include <cassert>

#define check(_Expression)    assert(_Expression); __assume(_Expression)

typedef unsigned __int8 byte; // 8bit int

#if __GNUC__
#define alignstruct(n) __attribute__(aligned(n))
#else
#define alignstruct(n) __declspec(align(n))
#endif

#if __GNUC__
#define forceinline __attribute((always_inline))
#else
#define forceinline __forceinline
#endif

#if __GNUC__
#define forcenoinline __attribute((never_inline))
#else
#define forcenoinline __declspec(noinline)
#endif

#if __GNUC__ && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6))
const
class nullptr_t
{
public:
	template<class T> // convertible to any type
	operator T*() const // of null non-member pointer...
	{
		return 0;
	}
	template<class C, class T> // or any type of null member pointer...
	operator T C::*() const
	{
		return 0;
	}
private:
	void operator&() const; // whose address can't be taken
} nullptr = {};
#endif

#ifndef SSE_VERSION
#if _M_X64
#define SSE_VERSION 3
#elif _M_IX86_FP >= 2
#define SSE_VERSION 2
#else
#define SSE_VERSION 0
#endif
#endif

#if SSE_VERSION >= 4
#include <smmintrin.h>
#elif SSE_VERSION >= 3
#include <pmmintrin.h>
#elif SSE_VERSION >= 2
#include <emmintrin.h>
//#include <xmmintrin.h>
#include <intrin.h>
#endif

#pragma warning (error:4715)
