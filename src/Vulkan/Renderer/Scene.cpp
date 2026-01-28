#include "Scene.h"
#include "AssetLoader.h"
#include "VulkanCore.h"
#include "VulkanSynchronization.h"

namespace tiny_vulkan {

	Scene::Scene()
	{
		// Prepare
		auto device = VulkanCore::GetDevice();

		std::filesystem::path wd = std::filesystem::current_path() / ".." / "src" / "EntryPoint" / "Assets";

		// Meshes
		m_Meshes = Loader::LoadGLTFMeshes(wd / "Gltf" / "KV2" / "kv-2_heavy_tank_1940.glb").value();

		// Shaders
		m_VertexShader = std::make_shared<VulkanShader>(wd / "Shaders" / "vertexShader.vert");
		m_FragmentShader = std::make_shared<VulkanShader>(wd / "Shaders" / "fragmentShader.frag");

		// Pipeline
		VkPushConstantRange pushRange;
		pushRange.offset = 0;
		pushRange.size = sizeof(ScenePushConstants);
		pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::vector<VkFormat> pipelineFormats = { VulkanCore::GetRenderTarget()->GetFormat() };

		m_Pipeline = VulkanPipelineBuilder()
			.SetPipelineType(PipelineType::GRAPHICS)
			.AddPushConstantRange(pushRange)
			.AddShader(m_VertexShader)
			.AddShader(m_FragmentShader)
			.SetColorAttachmentFormats(pipelineFormats)
			.SetDepthFormat(VK_FORMAT_UNDEFINED)
			.SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.Build();
	}

	void Scene::Render()
	{
		// Prepare
		auto cmdBuffer = VulkanCore::GetCurrentFrame()->GetCmdBuffer();
		auto rt = VulkanCore::GetRenderTarget();

		VkRenderingAttachmentInfo attachmentInfo = {};
		attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		attachmentInfo.pNext = nullptr;
		attachmentInfo.imageView = rt->GetView();
		attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
		attachmentInfo.resolveImageView = VK_NULL_HANDLE;
		attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // load operations define the initial values of an attachment during a render pass instance.
		attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // define how values written to an attachment during a render pass instance are stored to memory
		attachmentInfo.clearValue = {};

		VkExtent3D rtExtent = rt->GetExtent();
		VkRenderingInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = 0;
		renderingInfo.renderArea = { {0,0}, {rtExtent.width, rtExtent.height} };
		renderingInfo.layerCount = 1;
		renderingInfo.viewMask = 0;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &attachmentInfo;
		renderingInfo.pDepthAttachment = nullptr;
		renderingInfo.pStencilAttachment = nullptr;
		
		Synchronization::CmdImageMemoryBarrier(cmdBuffer, rt,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		// Begin rendering
		vkCmdBeginRendering(cmdBuffer, &renderingInfo);

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetRaw());

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = rtExtent.width;
		viewport.height = rtExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = rtExtent.width;
		scissor.extent.height = rtExtent.height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		for (auto mesh : m_Meshes)
		{
			// Constants
			glm::mat4 view = glm::translate(glm::vec3{ 0,0,-2 });
			glm::mat4 projection = glm::perspective(glm::radians(50.f), (float)1280 / (float)720, 0.1f, 10000.f);
			projection[1][1] *= -1;

			m_ScenePushConstants.vertexBufferAddress = mesh->vertexBufferAddress;
			m_ScenePushConstants.worldMatrix = projection * view;
			vkCmdPushConstants(cmdBuffer, m_Pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants), &m_ScenePushConstants);

			vkCmdBindIndexBuffer(cmdBuffer, mesh->indexBuffer->GetRaw(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(cmdBuffer, mesh->subMeshesGeo[0].count, 1, mesh->subMeshesGeo[0].startIndex, 0, 0);
		}

		vkCmdEndRendering(cmdBuffer);
	}

}