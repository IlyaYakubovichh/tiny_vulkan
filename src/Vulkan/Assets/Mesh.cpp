#include "Mesh.h"
#include "VulkanCore.h"
#include "CommandsExecutor.h"

#include <vk_mem_alloc.h>

namespace tiny_vulkan {

	std::shared_ptr<Mesh> Mesh::CreateMeshFrom(
		const std::string& name,
		const std::vector<Vertex>& vertices,
		const std::vector<uint32_t>& indices,
		const std::vector<SubMeshGeo>& subMeshesGeo)
	{
		const std::size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		const std::size_t indexBufferSize = indices.size() * sizeof(uint32_t);

		VmaAllocator allocator = VulkanCore::GetVmaAllocator();

		auto mesh = std::make_shared<Mesh>();
		mesh->name = name;
		mesh->subMeshesGeo = std::move(subMeshesGeo);

		// Allocate index and vertex buffers in GPU VRAM
		mesh->vertexBuffer = VulkanBufferBuilder()
			.SetAllocationPlace(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetAllocationSize(vertexBufferSize)
			.SetAllocator(allocator)
			.SetUsageMask(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
			.Build();

		mesh->indexBuffer = VulkanBufferBuilder()
			.SetAllocationPlace(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetAllocationSize(indexBufferSize)
			.SetAllocator(allocator)
			.SetUsageMask(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
			.Build();

		// Setup staging buffer
		auto stagingBuffer = VulkanBufferBuilder()
			.SetAllocationPlace(VMA_MEMORY_USAGE_CPU_ONLY)
			.SetAllocationSize(vertexBufferSize + indexBufferSize)
			.SetAllocator(allocator)
			.SetUsageMask(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.Build();

		// Copy content of vertex and index buffers into staging buffer
		void* data = nullptr;
		CHECK_VK_RES(vmaMapMemory(allocator, stagingBuffer->GetAllocation(), &data));

		memcpy(data, vertices.data(), vertexBufferSize);
		memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

		vmaUnmapMemory(allocator, stagingBuffer->GetAllocation());

		CommandExecutor::Execute(
			[&](VkCommandBuffer cmdBuffer)
			{
				// Copy vertex part from staging to vertex buffer in VRAM
				VkBufferCopy vertexBufferCopy = {};
				vertexBufferCopy.dstOffset = 0;
				vertexBufferCopy.srcOffset = 0;
				vertexBufferCopy.size = vertexBufferSize;
				vkCmdCopyBuffer(cmdBuffer, stagingBuffer->GetRaw(), mesh->vertexBuffer->GetRaw(), 1, &vertexBufferCopy);

				// Copy index part from staging to index buffer in VRAM
				VkBufferCopy indexBufferCopy = {};
				indexBufferCopy.dstOffset = 0;
				indexBufferCopy.srcOffset = vertexBufferSize;
				indexBufferCopy.size = indexBufferSize;
				vkCmdCopyBuffer(cmdBuffer, stagingBuffer->GetRaw(), mesh->indexBuffer->GetRaw(), 1, &indexBufferCopy);
			}
		);

		return mesh;
	}

}