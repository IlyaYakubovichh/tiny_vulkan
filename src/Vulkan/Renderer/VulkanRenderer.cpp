#include "Renderer/VulkanRenderer.h"
#include "VulkanCore.h"
#include "VulkanSynchronization.h"
#include "ImageOperations.h"

namespace tiny_vulkan {

	VulkanRenderer::VulkanRenderer(std::shared_ptr<Window> window)
		: m_Window(window)
	{
		m_Scene = std::make_shared<Scene>();
		m_ImGuiRenderer = std::make_shared<ImGuiRenderer>(m_Window);
	}

	void VulkanRenderer::Draw()
	{
		BeginFrame();

		m_Scene->Render();

		EndFrame();
	}

	void VulkanRenderer::BeginFrame()
	{
		auto	device				= VulkanCore::GetDevice();
		auto	swapchain			= VulkanCore::GetSwapchain()->GetRaw();
		auto&	frame				= VulkanCore::GetCurrentFrame();
		auto    renderFence			= frame->GetRenderFence();
		auto	imgAcqSemaphore		= frame->GetImageAcquireSemaphore();

		// Wait rendering finished
		CHECK_VK_RES(vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX));
		CHECK_VK_RES(vkResetFences(device, 1, &renderFence));

		// Acquire next swapchain image
		CHECK_VK_RES(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imgAcqSemaphore, VK_NULL_HANDLE, &m_CurrentImageIndex));

		CHECK_VK_RES(vkResetCommandBuffer(frame->GetCmdBuffer(), 0));

		// Prepare cmd buffer for commands recording
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		CHECK_VK_RES(vkBeginCommandBuffer(frame->GetCmdBuffer(), &beginInfo));
	}

	void VulkanRenderer::EndFrame()
	{
		auto&	frame			= VulkanCore::GetCurrentFrame();
		auto	cmdBuffer		= frame->GetCmdBuffer();
		auto	swImage			= VulkanCore::GetSwapchain()->GetImages()[m_CurrentImageIndex];
		auto	graphicsQueue	= VulkanCore::GetGraphicsQueue();
		auto	presentQueue	= VulkanCore::GetPresentQueue();
		auto	rawSwapchain	= VulkanCore::GetSwapchain()->GetRaw();

		CopyRenderTargetToSwapchain();
		DrawImGuiToSwapchain();

		Synchronization::CmdImageMemoryBarrier(cmdBuffer, swImage,
			VK_PIPELINE_STAGE_2_NONE,
			VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		CHECK_VK_RES(vkEndCommandBuffer(cmdBuffer));

		// Submit
		// Semaphore wait image available before output color
		VkSemaphoreSubmitInfo waitInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
		waitInfo.semaphore = frame->GetImageAcquireSemaphore();
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		// Semaphore signal after all graphics commands done
		VkSemaphoreSubmitInfo signalInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
		signalInfo.semaphore = frame->GetGlobalRenderSemaphores()[m_CurrentImageIndex];
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

		VkCommandBufferSubmitInfo cmdInfo = {};
		cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdInfo.commandBuffer = cmdBuffer;

		VkSubmitInfo2 submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdInfo;
		CHECK_VK_RES(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, frame->GetRenderFence()));

		// Present
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &rawSwapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &signalInfo.semaphore;
		presentInfo.pImageIndices = &m_CurrentImageIndex;
		CHECK_VK_RES(vkQueuePresentKHR(presentQueue, &presentInfo));

		VulkanCore::AdvanceFrame();
	}

	void VulkanRenderer::CopyRenderTargetToSwapchain()
	{
		auto  cmdBuffer  = VulkanCore::GetCurrentFrame()->GetCmdBuffer();
		auto  swImage	 = VulkanCore::GetSwapchain()->GetImages()[m_CurrentImageIndex];
		auto  rt		 = VulkanCore::GetRenderTarget();

		// Prepare rt for transfer src
		Synchronization::CmdImageMemoryBarrier(cmdBuffer, rt,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_ACCESS_2_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		// Prepare swapchain image for transfer dst
		Synchronization::CmdImageMemoryBarrier(cmdBuffer, swImage,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		// Copy rt to swapchain image
		ImageOperations::CmdBlit(cmdBuffer, rt, swImage, rt->GetExtent(), swImage->GetExtent());
	}

	void VulkanRenderer::DrawImGuiToSwapchain()
	{
		auto swapchainImage = VulkanCore::GetSwapchain()->GetImages()[m_CurrentImageIndex];

		m_ImGuiRenderer->DrawImGui(swapchainImage);
	}

}