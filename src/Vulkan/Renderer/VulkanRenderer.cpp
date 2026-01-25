#include "Renderer/VulkanRenderer.h"
#include "Commands/VulkanSynchronization.h"

namespace tiny_vulkan {

	VulkanRenderer::VulkanRenderer(std::shared_ptr<Window> window, std::shared_ptr<VulkanCore> core)
		: m_Window(window)
		, m_Core(core)
	{
		m_Frames.reserve(m_FlightFrameCount);
		for (int i = 0; i < m_FlightFrameCount; ++i)
		{
			m_Frames.push_back(std::make_shared<VulkanFrame>(
				m_Core->GetDevice(), 
				m_Core->GetGraphicsFamily(), 
				static_cast<uint32_t>(m_Core->GetSwapchain()->GetImages().size()))
			);
		}

		m_ImGuiRenderer = std::make_shared<ImGuiRenderer>(m_Window, m_Core);
	}

	void VulkanRenderer::Draw(Scene& scene)
	{
		auto& frame = m_Frames[m_CurrentFrameIndex];
		auto  cmd = frame->GetCmdBuffer();
		auto  rt = m_Core->GetRenderTarget();

		BeginFrame();

		// ...

		EndFrame();
	}

	void VulkanRenderer::BeginFrame()
	{
		auto device = m_Core->GetDevice();
		auto swapchain = m_Core->GetSwapchain()->GetRaw();
		auto& frame = m_Frames[m_CurrentFrameIndex];

		VkFence     renderFence = frame->GetRenderFence();
		VkSemaphore imgAcqSem = frame->GetImageAcquireSemaphore();

		// Wait rendering finished
		CHECK_VK_RES(vkWaitForFences(device, 1, &renderFence, VK_TRUE, UINT64_MAX));
		CHECK_VK_RES(vkResetFences(device, 1, &renderFence));

		// Acquire next swapchain image
		CHECK_VK_RES(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imgAcqSem, VK_NULL_HANDLE, &m_CurrentImageIndex));

		CHECK_VK_RES(vkResetCommandBuffer(frame->GetCmdBuffer(), 0));

		// Prepare cmd buffer for commands recording
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		CHECK_VK_RES(vkBeginCommandBuffer(frame->GetCmdBuffer(), &beginInfo));
	}

	void VulkanRenderer::EndFrame()
	{
		auto& frame		= m_Frames[m_CurrentFrameIndex];
		auto  cmd		= frame->GetCmdBuffer();
		auto  swImage	= m_Core->GetSwapchain()->GetImages()[m_CurrentImageIndex];
		auto  rt		= m_Core->GetRenderTarget();

		// ...

		CHECK_VK_RES(vkEndCommandBuffer(cmd));

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
		cmdInfo.commandBuffer = cmd;

		VkSubmitInfo2 submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdInfo;
		CHECK_VK_RES(vkQueueSubmit2(m_Core->GetGraphicsQueue(), 1, &submitInfo, frame->GetRenderFence()));

		// Present
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		VkSwapchainKHR swapchainRaw = m_Core->GetSwapchain()->GetRaw();
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchainRaw;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &signalInfo.semaphore;
		presentInfo.pImageIndices = &m_CurrentImageIndex;
		CHECK_VK_RES(vkQueuePresentKHR(m_Core->GetPresentQueue(), &presentInfo));

		// Advance frame (0,1,2,0,1,2,0 ....)
		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_FlightFrameCount;
	}

}