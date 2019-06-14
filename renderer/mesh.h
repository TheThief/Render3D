#pragma once

#include "math/vector_math.h"

#include <vector>

namespace renderer
{
	struct mesh
	{
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> tex_coords;

		std::vector<int> indices;
	};
}
