#pragma once

#include <array>

#include "math/vector_math.h"

constexpr float sqrt2_f = (float)M_SQRT2;

struct deferred_point_light_mesh
{
	std::array<Vector4, 20> positions =
	{{
		// back square
		{-(sqrt2_f - 1), -(sqrt2_f - 1), 1.0f},
		{-(sqrt2_f - 1),  (sqrt2_f - 1), 1.0f},
		{ (sqrt2_f - 1), -(sqrt2_f - 1), 1.0f},
		{ (sqrt2_f - 1),  (sqrt2_f - 1), 1.0f},

		// back octagon
		{-1,  (sqrt2_f-1), (sqrt2_f-1)},
		{-(sqrt2_f-1),  1, (sqrt2_f-1)},
		{ (sqrt2_f-1),  1, (sqrt2_f-1)},
		{ 1,  (sqrt2_f-1), (sqrt2_f-1)},
		{ 1, -(sqrt2_f-1), (sqrt2_f-1)},
		{ (sqrt2_f-1), -1, (sqrt2_f-1)},
		{-(sqrt2_f-1), -1, (sqrt2_f-1)},
		{-1, -(sqrt2_f-1), (sqrt2_f-1)},

		// front octagon
		{-1, -(sqrt2_f-1), -1},
		{-1,  (sqrt2_f-1), -1},
		{-(sqrt2_f-1),  1, -1},
		{ (sqrt2_f-1),  1, -1},
		{ 1,  (sqrt2_f-1), -1},
		{ 1, -(sqrt2_f-1), -1},
		{ (sqrt2_f-1), -1, -1},
		{-(sqrt2_f-1), -1, -1}
	}};

	std::array<int, 102> indices =
	{{
		// back square
		0, 1, 2,
		2, 1, 3,

		// back octagon
		4, 5, 0,
		0, 5, 1,
		5, 6, 1,
		1, 6, 3,
		3, 6, 7,
		3, 7, 8,
		2, 3, 9,
		9, 3, 8,
		10, 2, 9,
		11, 0, 10,
		10, 0, 2,
		11, 4, 0,

		// front octagon
		12, 13, 4,
		4,  13, 5,
		13, 14, 5,
		5,  14, 6,
		6,  14, 7,
		7,  14, 15,
		8,  7,  16,
		16, 7,  15,
		9,  8,  17,
		17, 8,  16,
		18, 10, 9,
		18, 9,  17,
		19, 11, 18,
		18, 11, 10,
		19, 12, 11,
		11, 12, 4,
	}};
};