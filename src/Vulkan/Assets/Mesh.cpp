#include "Mesh.h"
#include "VulkanCore.h"
#include "CommandsExecutor.h"
#include "LifetimeManager.h"

#include <vk_mem_alloc.h>

namespace tiny_vulkan {

	std::shared_ptr<Mesh> Mesh::CreateMeshFrom(
		const std::string& name,
		const std::span<Vertex>& vertices,
		const std::span<uint32_t>& indices,
		const std::vector<SubMeshGeo>& subMeshesGeo)
	{
		auto device = VulkanCore::GetDevice();

		const std::size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		const std::size_t indexBufferSize = indices.size() * sizeof(uint32_t);

		VmaAllocator allocator = VulkanCore::GetVmaAllocator();

		auto mesh = std::make_shared<Mesh>();
		mesh->name = name;
		mesh->subMeshesGeo = subMeshesGeo;

		// Allocate index and vertex buffers in GPU VRAM
		// Vertex buffer
		mesh->vertexBuffer = VulkanBufferBuilder()
			.SetAllocationPlace(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetAllocationSize(vertexBufferSize)
			.SetUsageMask(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT)
			.Build();

		VkBufferDeviceAddressInfo addressInfo = {};
		addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		addressInfo.pNext = nullptr;
		addressInfo.buffer = mesh->vertexBuffer->GetRaw();
		mesh->vertexBufferAddress = vkGetBufferDeviceAddress(device, &addressInfo);

		LifetimeManager::PushFunction(vmaDestroyBuffer, allocator, mesh->vertexBuffer->GetRaw(), mesh->vertexBuffer->GetAllocation());

		// Index buffer
		mesh->indexBuffer = VulkanBufferBuilder()
			.SetAllocationPlace(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetAllocationSize(indexBufferSize)
			.SetUsageMask(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
			.Build();
		LifetimeManager::PushFunction(vmaDestroyBuffer, allocator, mesh->indexBuffer->GetRaw(), mesh->indexBuffer->GetAllocation());

		// Setup staging buffer
		auto stagingBuffer = VulkanBufferBuilder()
			.SetAllocationPlace(VMA_MEMORY_USAGE_CPU_ONLY)
			.SetAllocationSize(vertexBufferSize + indexBufferSize)
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

				LifetimeManager::ExecuteNow(vmaDestroyBuffer, allocator, stagingBuffer->GetRaw(), stagingBuffer->GetAllocation());
			}
		);

		return mesh;
	}

}