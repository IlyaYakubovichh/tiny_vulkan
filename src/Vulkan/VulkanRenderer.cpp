#include "VulkanRenderer.h"
#include "VulkanSync.h"
#include "LogSystem.h"
#include "LifetimeManager.h"
#include "VulkanUtils.h"

namespace tiny_vulkan {

	std::shared_ptr<Window>     VulkanRenderer::s_Window;
	std::shared_ptr<VulkanCore> VulkanRenderer::s_VulkanCore;
	std::vector<std::shared_ptr<VulkanFrame>> VulkanRenderer::s_Frames;

	uint32_t VulkanRenderer::s_CurrentFrameIndex = 0;
	uint32_t VulkanRenderer::s_CurrentImageIndex = 0;

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
	}

	void VulkanRenderer::Shutdown()
	{
		if (s_VulkanCore) {
			vkDeviceWaitIdle(s_VulkanCore->GetDevice());
		}

		s_Frames.clear();

		s_VulkanCore.reset();
		s_Window.reset();

		LifetimeManager::ExecuteAll();
	}

	void VulkanRenderer::BeginFrame()
	{
		auto device = s_VulkanCore->GetDevice();
		auto swapchain = s_VulkanCore->GetSwapchain()->GetRaw();
		auto& frame = s_Frames[s_CurrentFrameIndex];

		// Wait for the previous frame to finish
		VkFence renderFence = frame->GetRenderFence();
		CHECK_VK_RES(vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX));

		// Acquire next image
		VkSemaphore imageSemaphore = frame->GetImageAcquireSemaphore();
		CHECK_VK_RES(vkAcquireNextImageKHR(
			device,
			swapchain,
			UINT64_MAX,
			imageSemaphore,
			VK_NULL_HANDLE,
			&s_CurrentImageIndex
		));

		CHECK_VK_RES(vkResetFences(device, 1, &renderFence));

		// Reset Command Buffer
		auto cmdBuffer = frame->GetCmdBuffer();
		CHECK_VK_RES(vkResetCommandBuffer(cmdBuffer, 0));

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CHECK_VK_RES(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
	}

	void VulkanRenderer::EndFrame()
	{
		auto& frame = s_Frames[s_CurrentFrameIndex];
		auto cmdBuffer = frame->GetCmdBuffer();

		// Transition Swapchain Image for Presentation
		auto swapchainImage = s_VulkanCore->GetSwapchain()->GetImages()[s_CurrentImageIndex];

		VulkanSync::InsertImageMemoryBarrier(
			cmdBuffer,
			swapchainImage,
			VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);

		CHECK_VK_RES(vkEndCommandBuffer(cmdBuffer));

		// Submit
		auto graphicsQueue = s_VulkanCore->GetGraphicsQueue();
		auto imageAcquireSem = frame->GetImageAcquireSemaphore();
		auto renderFinishedSem = frame->GetRenderSemaphores()[s_CurrentImageIndex];
		auto renderFence = frame->GetRenderFence();

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

		// Present
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
		auto swapchainImages = s_VulkanCore->GetSwapchain()->GetImages();
		auto image = swapchainImages[s_CurrentImageIndex];

		// Transition to Transfer Dst
		VulkanSync::InsertImageMemoryBarrier(
			cmd,
			image,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		VkClearColorValue clearColor = { {color.r, color.g, color.b, 1.0f} };

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = 1;
		range.levelCount = 1;

		vkCmdClearColorImage(cmd, image->GetRaw(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
	}

	void VulkanRenderer::OnUpdate()
	{
		BeginFrame();

		auto cmdBuffer = s_Frames[s_CurrentFrameIndex]->GetCmdBuffer();
		Clear(cmdBuffer, glm::vec3(1.0f, 0.0f, 0.0f));

		EndFrame();
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