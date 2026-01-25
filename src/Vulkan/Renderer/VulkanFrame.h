#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace tiny_vulkan {

	class VulkanFrame
	{
	public:
		explicit VulkanFrame(VkDevice device, uint32_t queueFamilyIndex, uint32_t swapchainImageCount);
		~VulkanFrame() = default;

		VulkanFrame(const VulkanFrame&) = delete;
		VulkanFrame& operator=(const VulkanFrame&) = delete;

		[[nodiscard]] static const std::vector<VkSemaphore>& GetGlobalRenderSemaphores() { return s_RenderSemaphores; }

		[[nodiscard]] VkCommandPool   GetPool()                  const { return m_Pool; }
		[[nodiscard]] VkCommandBuffer GetCmdBuffer()             const { return m_CmdBuffer; }
		[[nodiscard]] VkSemaphore     GetImageAcquireSemaphore() const { return m_ImageAcquireSemaphore; }
		[[nodiscard]] VkFence         GetRenderFence()           const { return m_RenderFence; }

	private:
		static std::vector<VkSemaphore> s_RenderSemaphores;

		VkCommandPool		m_Pool{ VK_NULL_HANDLE };
		VkCommandBuffer		m_CmdBuffer{ VK_NULL_HANDLE };
		VkSemaphore			m_ImageAcquireSemaphore{ VK_NULL_HANDLE };
		VkFence				m_RenderFence{ VK_NULL_HANDLE };
	};

}