#include "RendererData.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "LifetimeManager.h"
#include <filesystem>

namespace tiny_vulkan {

	void RendererData::Initialize()
	{
		InitializeImmediate();
		PrepareData();
		CreatePipelines();
	}

	MeshBuffers RendererData::UploadMesh(std::span<Vertex> vertices, std::span<uint32_t> indices)
	{
		auto device = VulkanRenderer::GetCore()->GetDevice();

		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

		MeshBuffers meshBuffers;

		// Vertex buffer
		meshBuffers.vertexBuffer = VulkanBufferBuilder()
			.SetAllocationSize(vertexBufferSize)
			.SetAllocationPlace(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetUsageMask(
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT	| 
				VK_BUFFER_USAGE_TRANSFER_DST_BIT	|
				VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
			.Build();

		// Device address
		VkBufferDeviceAddressInfo addressInfo = {};
		addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		addressInfo.pNext = nullptr;
		addressInfo.buffer = meshBuffers.vertexBuffer->GetRaw();
		meshBuffers.vertexBufferAddress = vkGetBufferDeviceAddress(device, &addressInfo);

		// Index buffer
		meshBuffers.indexBuffer = VulkanBufferBuilder()
			.SetAllocationSize(vertexBufferSize)
			.SetAllocationPlace(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetUsageMask(
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | 
				VK_BUFFER_USAGE_TRANSFER_DST_BIT)
			.Build();

		// Staging buffer
		auto stagingBuffer = VulkanBufferBuilder()
			.SetAllocationSize(vertexBufferSize + indexBufferSize)
			.SetAllocationPlace(VMA_MEMORY_USAGE_CPU_ONLY)
			.SetUsageMask(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.Build();

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(VulkanRenderer::GetCore()->GetVmaAllocator(), stagingBuffer->GetAllocation(), &allocInfo);

		void* data;
		vmaMapMemory(VulkanRenderer::GetCore()->GetVmaAllocator(), stagingBuffer->GetAllocation(), &data);

		memcpy(data, vertices.data(), vertexBufferSize);
		memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

		vmaUnmapMemory(VulkanRenderer::GetCore()->GetVmaAllocator(), stagingBuffer->GetAllocation());

		// Copy from staging buffer to: 1) vertexBuffer  2) indexBuffer
		VulkanRenderer::ImmediateSubmit([&](VkCommandBuffer cmdBuffer) -> void
			{
				VkBufferCopy vertexCopy = {};
				vertexCopy.srcOffset = 0;
				vertexCopy.dstOffset = 0;
				vertexCopy.size = vertexBufferSize;
				vkCmdCopyBuffer(cmdBuffer, stagingBuffer->GetRaw(), meshBuffers.vertexBuffer->GetRaw(), 1, &vertexCopy);

				VkBufferCopy indexCopy = {};
				indexCopy.srcOffset = vertexBufferSize;
				indexCopy.dstOffset = 0;
				indexCopy.size = indexBufferSize;
				vkCmdCopyBuffer(cmdBuffer, stagingBuffer->GetRaw(), meshBuffers.indexBuffer->GetRaw(), 1, &indexCopy);
			});

		return meshBuffers;
	}

	void RendererData::CreatePipelines()
	{
		// Shaders
		std::filesystem::path wd = std::filesystem::current_path() / ".." / "src" / "EntryPoint" / "Assets";
		m_VertexShader = std::make_shared<VulkanShader>(wd / "triangleV.vert");
		m_FragmentShader = std::make_shared<VulkanShader>(wd / "triangleF.frag");

		// Pipeline
		std::vector<VkFormat> colorAttachmentFormats;
		colorAttachmentFormats.push_back(VulkanRenderer::GetCore()->GetRenderTarget()->GetFormat());

		VkPushConstantRange range = {};
		range.offset = 0;
		range.size = sizeof(PushConstants);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		m_MeshPipeline = VulkanPipelineBuilder()
			.SetPipelineType(PipelineType::GRAPHICS)
			.AddShader(m_VertexShader)
			.AddShader(m_FragmentShader)
			.LayoutAddPushRange(range)
			.SetColorAttachmentFormats(colorAttachmentFormats)
			.SetDepthFormat(VK_FORMAT_UNDEFINED)
			.SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.Build();
	}

	void RendererData::InitializeImmediate()
	{
		auto device = VulkanRenderer::GetCore()->GetDevice();
		uint32_t graphicsFamily = VulkanRenderer::GetCore()->GetGraphicsFamily();

		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = graphicsFamily;
		CHECK_VK_RES(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &m_Pool));

		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = m_Pool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAllocInfo.commandBufferCount = 1;
		CHECK_VK_RES(vkAllocateCommandBuffers(device, &cmdAllocInfo, &m_ImmediateCmdBuffer));

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		CHECK_VK_RES(vkCreateFence(device, &fenceInfo, nullptr, &m_ImmediateFence));

		LifetimeManager::PushFunction(vkDestroyCommandPool, device, m_Pool, nullptr);
		LifetimeManager::PushFunction(vkDestroyFence, device, m_ImmediateFence, nullptr);
	}

	void RendererData::PrepareData()
	{
		std::filesystem::path wd = std::filesystem::current_path() / ".." / "src" / "EntryPoint" / "Assets";
		m_Meshes = Loader::LoadGltfMeshes(wd / "basicmesh.glb").value();
	}

}