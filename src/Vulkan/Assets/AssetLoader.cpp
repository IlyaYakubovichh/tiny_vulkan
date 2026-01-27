#include "AssetLoader.h"
#include "LogSystem.h"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

namespace tiny_vulkan::Loader {

	std::optional<std::vector<std::shared_ptr<Mesh>>> LoadGLTFMeshes(std::filesystem::path filepath)
	{
		LOG_INFO(fmt::runtime("Loading file: {}"), filepath.string());

		// Load .glb file
		fastgltf::GltfDataBuffer gltfData;
		gltfData.FromPath(filepath);

		// Options
		constexpr auto gltfOptions = 
			fastgltf::Options::LoadGLBBuffers | // load bin chunk into ram
			fastgltf::Options::LoadExternalBuffers; // load external buffers referenced by URI into ram

		// Load glb file and construct Asset
		fastgltf::Asset gltf;
		fastgltf::Parser parser{};
		auto load = parser.loadGltfBinary(gltfData, filepath.parent_path(), gltfOptions);
		if (load)
		{
			gltf = std::move(load.get());
		}
		else
		{
			LOG_ERROR(fmt::runtime("Failed to load gltf binary: {}"), filepath.string());
			return {};
		}

		// Preparing meshes
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::vector<uint32_t> indices;
		std::vector<Vertex> vertices;
		std::vector<SubMeshGeo> subMeshesGeo;

		for (fastgltf::Mesh& mesh : gltf.meshes)
		{
			vertices.clear();
			indices.clear();
			subMeshesGeo.clear();

			for (auto& primitive : mesh.primitives)
			{
				SubMeshGeo subMeshGeo;
				subMeshGeo.startIndex = (uint32_t) indices.size();
				subMeshGeo.count = (uint32_t) gltf.accessors[primitive.indicesAccessor.value()].count;

				uint32_t vertexBufferStartPoint = vertices.size();

				// load indices
				fastgltf::Accessor& indicesAccessor = gltf.accessors[primitive.indicesAccessor.value()];
				indices.reserve(indices.size() + indicesAccessor.count);

				fastgltf::iterateAccessor<uint32_t>(gltf, indicesAccessor, 
					[&](std::uint32_t index)
					{
						indices.push_back(index + vertexBufferStartPoint);
					}
				);

				// load vertices
				fastgltf::Accessor& verticesAccessor = gltf.accessors[primitive.findAttribute("POSITION")->accessorIndex]; // get accessor index
				vertices.resize(vertices.size() + verticesAccessor.count);

				fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, verticesAccessor, 
					[&](glm::vec3 vertex, size_t index)
					{
						Vertex newVertex;
						newVertex.position = vertex;
						newVertex.normal = glm::vec3(1.0f, 0.0f, 0.0f);
						newVertex.color = glm::vec4(1.0f);
						newVertex.uv_x = 0;
						newVertex.uv_y = 0;
						vertices[index + vertexBufferStartPoint] = newVertex;
					});

				// load vertex normals
				fastgltf::Accessor& normalsAccessor = gltf.accessors[primitive.findAttribute("NORMAL")->accessorIndex];

				fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, normalsAccessor,
					[&](glm::vec3 normal, size_t index)
					{
						vertices[index + vertexBufferStartPoint].normal = normal;
					}
				);

				// load UVs
				fastgltf::Accessor& uvAccessor = gltf.accessors[primitive.findAttribute("TEXCOORD_0")->accessorIndex];

				fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, uvAccessor,
					[&](glm::vec2 uv, size_t index)
					{
						vertices[index + vertexBufferStartPoint].uv_x = uv.x;
						vertices[index + vertexBufferStartPoint].uv_y = uv.y;
					}
				);

				// load color
				fastgltf::Accessor& colorAccessor = gltf.accessors[primitive.findAttribute("COLOR_0")->accessorIndex];

				fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, colorAccessor,
					[&](glm::vec4 color, size_t index) 
					{
						vertices[index + vertexBufferStartPoint].color = color;
					}
				);

				subMeshesGeo.push_back(subMeshGeo);
			}

			// Vertex and index buffers are ready, creating mesh
			meshes.push_back(Mesh::CreateMeshFrom(std::string(mesh.name), vertices, indices, subMeshesGeo));
		}
	}

}