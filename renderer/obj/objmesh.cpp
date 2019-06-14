#include "objmesh.h"

#include "tiny_obj_loader/tiny_obj_loader.h"

#include <fstream>
#include <filesystem>

#include <range/v3/core.hpp>
#include <range/v3/action/transform.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/transform.hpp>

namespace renderer
{
	objmesh load_obj(std::filesystem::path file_path, bool coalesce_shapes)
	{
		objmesh result;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		std::ifstream file_stream(file_path);
		tinyobj::MaterialFileReader material_reader(std::filesystem::path(file_path).remove_filename().string() += '/');
		bool success = tinyobj::LoadObj(&attrib, &shapes,
			&materials, &err,
			&file_stream, &material_reader,
			true);

		if (!success)
		{
			throw err;
		}

		std::map<int, size_t> matid_to_element_index;

		for (const tinyobj::shape_t& shape : shapes)
		{
			if (!coalesce_shapes)
			{
				matid_to_element_index.clear();
			}

			for (size_t face_index = 0, index_index = 0; face_index < shape.mesh.num_face_vertices.size(); ++face_index)
			{
				const int material_id = shape.mesh.material_ids[face_index];
				auto [found_element_index, new_element] = matid_to_element_index.try_emplace(shape.mesh.material_ids[face_index], result.elements.size());
				if (new_element)
				{
					// material not used yet, need to add a new element
					objmesh::element new_element;
					if (material_id != -1)
					{
						const tinyobj::material_t& material = materials[material_id];
						new_element.diffuse_colour.R = material.diffuse[0];
						new_element.diffuse_colour.G = material.diffuse[1];
						new_element.diffuse_colour.B = material.diffuse[2];
						new_element.diffuse_texture = material.diffuse_texname;
					}
					result.elements.push_back(std::move(new_element));
				}

				objmesh::element& current_element = result.elements[found_element_index->second];
				assert(shape.mesh.num_face_vertices[face_index] == 3);
				const auto end_index_index = index_index + shape.mesh.num_face_vertices[face_index];
				for (; index_index < end_index_index; ++index_index)
				{
					const tinyobj::index_t& obj_face_indices = shape.mesh.indices[index_index];
					const tinyobj::real_t* const obj_vertex = &attrib.vertices[obj_face_indices.vertex_index * 3];
					const Vector3 position = { obj_vertex[0], obj_vertex[1], obj_vertex[2] };
					Vector3 normal = { 0, 0, 0 };
					Vector2 tex_coord = { 0, 0 };

					if (obj_face_indices.normal_index != -1)
					{
						const tinyobj::real_t* const obj_normal = &attrib.normals[obj_face_indices.normal_index * 3];
						normal = { obj_normal[0], obj_normal[1], obj_normal[2] };
					}

					if (obj_face_indices.texcoord_index != -1)
					{
						const tinyobj::real_t* const obj_texcoord = &attrib.texcoords[obj_face_indices.texcoord_index * 2];
						tex_coord = { obj_texcoord[0], obj_texcoord[1] };
					}

					size_t vertex_index = 0;
					for (; vertex_index < current_element.mesh.positions.size(); ++vertex_index)
					{
						if (current_element.mesh.positions[vertex_index] == position)
						{
							if (obj_face_indices.normal_index == -1 ||
								current_element.mesh.normals[vertex_index] == normal)
							{
								if (obj_face_indices.texcoord_index == -1 ||
									current_element.mesh.tex_coords[vertex_index] == tex_coord)
								{
									break;
								}
							}
						}
					}
					if (vertex_index == current_element.mesh.positions.size())
					{
						current_element.mesh.positions.push_back(position);
						//if (obj_face_indices.normal_index != -1)
						//{
						//	current_element.mesh.normals.resize(vertex_index, { 0, 0, 0 });
							current_element.mesh.normals.push_back(normal);
						//}
						//if (obj_face_indices.texcoord_index != -1)
						//{
						//	current_element.mesh.tex_coords.resize(vertex_index, { 0, 0 });
							current_element.mesh.tex_coords.push_back(tex_coord);
						//}
					}

					current_element.mesh.indices.push_back((int)vertex_index);
				}
			}
		}

		// calculate normals
		if (attrib.normals.size() == 0)
		{
			for (auto& current_element : result.elements)
			{
				for (auto face_vert_indices : current_element.mesh.indices | ranges::view::chunk(3))
				{
					std::array<Vector3, 3> face_vert_positions;
					ranges::transform(face_vert_indices, face_vert_positions.begin(), [&](int index) { return current_element.mesh.positions[index]; });
					Vector3 a = (face_vert_positions[1] - face_vert_positions[0]);
					Vector3 b = (face_vert_positions[2] - face_vert_positions[0]);
					Vector3 face_normal = a.Cross(b);
					ranges::for_each(face_vert_indices, [&](int index) { current_element.mesh.normals[index] += face_normal; });
				}
				ranges::action::transform(current_element.mesh.normals, &Vector3::Normalized);
			}
		}

		return result;
	}
}
