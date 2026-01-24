#include "VulkanRenderer.h"
#include "ImGui/ImGuiRenderer.h"
#include "VulkanSync.h"
#include "VulkanTransfer.h"
#include "LifetimeManager.h"
#include "VulkanUtils.h"
#include "LogSystem.h"
#include <filesystem>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace tiny_vulkan {

	// ========================================================
	// Static init
	// ========================================================
	std::shared_ptr<RendererData>	VulkanRenderer::s_Data;
	std::shared_ptr<Window>			VulkanRenderer::s_Window;
	std::shared_ptr<VulkanCore>		VulkanRenderer::s_VulkanCore;

	std::vector<std::shared_ptr<VulkanFrame>> VulkanRenderer::s_Frames;

	uint32_t VulkanRenderer::s_CurrentFrameIndex = 0;
	uint32_t VulkanRenderer::s_CurrentImageIndex = 0;

	// ========================================================
	// Implementation
	// ========================================================
	void VulkanRenderer::Initialize()
	{
		LogSystem::Initialize();

		s_Window = std::make_unique<Window>();
		s_VulkanCore = std::make_unique<VulkanCore>();

		s_Frames.reserve(FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i)
		{
			s_Frames.push_back(std::make_unique<VulkanFrame>(
				s_VulkanCore->GetDevice(),
				s_VulkanCore->GetGraphicsFamily())
			);
		}

		s_Data = std::make_shared<RendererData>();
		s_Data->Initialize();

		ImGuiRenderer::Initialize();
	}

	void VulkanRenderer::Shutdown()
	{
		if (s_VulkanCore) 
		{
			vkDeviceWaitIdle(s_VulkanCore->GetDevice());
		}

		s_Frames.clear();

		s_VulkanCore.reset();
		s_Window.reset();

		LifetimeManager::ExecuteAll();
	}

	void VulkanRenderer::BeginFrame()
	{
		// ========================================================
		// Required data
		// ========================================================
		auto		device			= s_VulkanCore->GetDevice();
		auto		swapchain		= s_VulkanCore->GetSwapchain()->GetRaw();
		auto&		frame			= s_Frames[s_CurrentFrameIndex];
		auto		cmdBuffer		= frame->GetCmdBuffer();
		VkFence		renderFence		= frame->GetRenderFence();
		VkSemaphore imageSemaphore	= frame->GetImageAcquireSemaphore();

		// ========================================================
		// Begin
		// ========================================================
		// Wait prev render finished
		CHECK_VK_RES(vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX));

		// Acquire next image
		CHECK_VK_RES(vkAcquireNextImageKHR(
			device,
			swapchain,
			UINT64_MAX,
			imageSemaphore,
			VK_NULL_HANDLE,
			&s_CurrentImageIndex
		));

		// Reset and begin Command Buffer
		CHECK_VK_RES(vkResetFences(device, 1, &renderFence));

		CHECK_VK_RES(vkResetCommandBuffer(cmdBuffer, 0));

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CHECK_VK_RES(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
	}

	void VulkanRenderer::EndFrame()
	{
		auto&	frame			= s_Frames[s_CurrentFrameIndex];
		auto	cmdBuffer		= frame->GetCmdBuffer();
		auto	swapchainImage	= s_VulkanCore->GetSwapchain()->GetImages()[s_CurrentImageIndex];
		auto	renderTarget	= s_VulkanCore->GetRenderTarget();

		// ========================================================
		// Barriers + Finish
		// ========================================================
		// Calculate imgui state
		ImGuiRenderer::CalculateInternal();

		// Blit render target to swapchain image
		VulkanTransfer::Blit( // Handles sync internally
			cmdBuffer, 
			renderTarget, 
			swapchainImage,
			renderTarget->GetExtent(),
			swapchainImage->GetExtent());

		// Prepare swapchain image for imgui
		VulkanSync::InsertImageMemoryBarrier( 
			cmdBuffer,
			swapchainImage,
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		// Draw imgui into swapchain image
		ImGuiOnUpdatePackage package;
		package.cmdBuffer	= cmdBuffer;
		package.extent		= swapchainImage->GetExtent();
		package.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		package.imageView	= swapchainImage->GetView();

		ImGuiRenderer::OnUpdate(package);

		// Prepare swapchain image for presentation
		VulkanSync::InsertImageMemoryBarrier(
			cmdBuffer,
			swapchainImage,
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);

		CHECK_VK_RES(vkEndCommandBuffer(cmdBuffer));

		// ========================================================
		// Submit
		// ========================================================
		auto graphicsQueue		= s_VulkanCore->GetGraphicsQueue();
		auto imageAcquireSem	= frame->GetImageAcquireSemaphore();
		auto renderFinishedSem	= frame->GetRenderSemaphores()[s_CurrentImageIndex];
		auto renderFence		= frame->GetRenderFence();

		VkCommandBufferSubmitInfo cmdInfo = {};
		cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdInfo.commandBuffer = cmdBuffer;

		VkSemaphoreSubmitInfo waitInfo = {};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.semaphore = imageAcquireSem;
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSemaphoreSubmitInfo signalInfo = {};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.semaphore = renderFinishedSem;
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

		VkSubmitInfo2 submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdInfo;

		CHECK_VK_RES(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, renderFence));

		// ========================================================
		// Present
		// ========================================================
		auto presentQueue = s_VulkanCore->GetPresentQueue();
		auto swapchainRaw = s_VulkanCore->GetSwapchain()->GetRaw();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchainRaw;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSem;
		presentInfo.pImageIndices = &s_CurrentImageIndex;

		CHECK_VK_RES(vkQueuePresentKHR(presentQueue, &presentInfo));

		// Advance Frame
		s_CurrentFrameIndex = (s_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::Clear(VkCommandBuffer cmd, glm::vec3 color)
	{
		auto renderTarget = s_VulkanCore->GetRenderTarget();

		// ========================================================
		// Barriers
		// ========================================================
		VulkanSync::InsertImageMemoryBarrier(
			cmd,
			renderTarget,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		// ========================================================
		// Clear
		// ========================================================
		VkClearColorValue clearColor = { {color.r, color.g, color.b, 1.0f} };

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = 1;
		range.levelCount = 1;

		vkCmdClearColorImage(cmd, renderTarget->GetRaw(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
	}

	void VulkanRenderer::Dispatch(VkCommandBuffer cmdBuffer, uint32_t groupX, uint32_t groupY, uint32_t groupZ)
	{
		VulkanSync::InsertImageMemoryBarrier(
			cmdBuffer, 
			s_VulkanCore->GetRenderTarget(),
			VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
			VK_ACCESS_2_SHADER_WRITE_BIT,
			VK_IMAGE_LAYOUT_GENERAL);

		vkCmdDispatch(cmdBuffer, groupX, groupY, groupZ);
	}

	void VulkanRenderer::DrawRectangle(VkCommandBuffer cmdBuffer)
	{
		auto renderTarget = s_VulkanCore->GetRenderTarget();
		VkExtent3D renderTargetExtent = renderTarget->GetExtent();

		VkRenderingAttachmentInfo attachmentInfo = {};
		attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		attachmentInfo.pNext = nullptr;
		attachmentInfo.imageView = renderTarget->GetView();
		attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = 0;
		renderingInfo.renderArea = VkRect2D{ {0,0}, {renderTargetExtent.width, renderTargetExtent.height} };
		renderingInfo.layerCount = 1;
		renderingInfo.viewMask = 0;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &attachmentInfo;
		renderingInfo.pDepthAttachment = nullptr;
		renderingInfo.pStencilAttachment = nullptr;

		// ========================================================
		// Sync
		// ========================================================
		VulkanSync::InsertImageMemoryBarrier(
			cmdBuffer,
			renderTarget,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		// ========================================================
		// Begin render pass
		// ========================================================
		vkCmdBeginRendering(cmdBuffer, &renderingInfo);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = renderTargetExtent.width;
		viewport.height = renderTargetExtent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = renderTargetExtent.width;
		scissor.extent.height = renderTargetExtent.height;
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_Data->m_MeshPipeline->GetRaw());

		glm::mat4 view = glm::translate(glm::vec3{0,0,-5});
		glm::mat4 projection = glm::perspective(
			glm::radians(40.f),       // FOV
			(float)1280 / (float)720, // Aspect Ratio
			0.1f,                     // zNear 
			10000.f                   // zFar 
		);
		projection[1][1] *= -1; 

		PushConstants push_constants;
		push_constants.worldMatrix = projection * view;
		push_constants.vertexBufferAddress = s_Data->m_Meshes[2]->meshBuffers.vertexBufferAddress;
		vkCmdPushConstants(cmdBuffer, s_Data->m_MeshPipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &push_constants);

		vkCmdBindIndexBuffer(cmdBuffer, s_Data->m_Meshes[2]->meshBuffers.indexBuffer->GetRaw(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, s_Data->m_Meshes[2]->surfaces[0].count, 1, s_Data->m_Meshes[2]->surfaces[0].startIndex, 0, 0);

		vkCmdEndRendering(cmdBuffer);
	}

	void VulkanRenderer::OnUpdate()
	{
		BeginFrame();

		// ========================================================
		// Prepare
		// ========================================================
		auto  cmdBuffer	= s_Frames[s_CurrentFrameIndex]->GetCmdBuffer();

		// ========================================================
		// Draw
		// ========================================================
		DrawRectangle(cmdBuffer);

		// ========================================================
		// End
		// ========================================================
		EndFrame();
	}

	void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmdBuffer)>&& func)
	{
		auto device = s_VulkanCore->GetDevice();
		VkCommandBuffer immediateCommandBuffer = s_Data->m_ImmediateCmdBuffer;
		VkFence immediateFence = s_Data->m_ImmediateFence;
		VkQueue graphicsQueue = s_VulkanCore->GetGraphicsQueue();

		CHECK_VK_RES(vkResetFences(device, 1, &immediateFence));

		CHECK_VK_RES(vkResetCommandBuffer(immediateCommandBuffer, 0));

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;
		CHECK_VK_RES(vkBeginCommandBuffer(immediateCommandBuffer, &beginInfo));

		func(immediateCommandBuffer);

		vkEndCommandBuffer(immediateCommandBuffer);

		VkCommandBufferSubmitInfo cmdBufferSubmitInfo = {};
		cmdBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdBufferSubmitInfo.pNext = nullptr;
		cmdBufferSubmitInfo.deviceMask = 0;
		cmdBufferSubmitInfo.commandBuffer = immediateCommandBuffer;

		VkSubmitInfo2 submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.waitSemaphoreInfoCount = 0;
		submitInfo.pWaitSemaphoreInfos = nullptr;
		submitInfo.signalSemaphoreInfoCount = 0;
		submitInfo.pSignalSemaphoreInfos = nullptr;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdBufferSubmitInfo;
		CHECK_VK_RES(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, immediateFence));

		vkWaitForFences(device, 1, &immediateFence, VK_TRUE, UINT32_MAX);
	}

	void VulkanRenderer::Run()
	{
		Initialize();

		while (!s_Window->ShouldClose())
		{
			s_Window->OnUpdate();
			OnUpdate();
		}

		Shutdown(); 
	}
}