#pragma once

#include "renderer/mesh.h"
#include "math/colour.h"

#include <vector>
#include <filesystem>

namespace std::filesystem
{
	using namespace std::experimental::filesystem;
}

namespace renderer
{
	struct objmesh
	{
		struct element
		{
			renderer::mesh mesh;
			std::filesystem::path diffuse_texture;
			fRGBA diffuse_colour = {1,1,1,1};
		};

		std::vector<element> elements;
	};

	objmesh load_obj(std::filesystem::path file_path, bool coalesce_shapes = true);
}
