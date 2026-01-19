#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace tiny_vulkan {

	class VulkanFrame
	{
	public:
		VulkanFrame(VkDevice device, uint32_t queueFamilyIndex);
		~VulkanFrame() = default;

		[[nodiscard]] static auto& GetRenderSemaphores() { return s_RenderSemaphores; }
		[[nodiscard]] auto GetPool()					const { return m_Pool; }
		[[nodiscard]] auto GetCmdBuffer()				const { return m_CmdBuffer; }
		[[nodiscard]] auto GetImageAcquireSemaphore()	const { return m_ImageAcquireSemaphore; }
		[[nodiscard]] auto GetRenderFence()				const { return m_RenderFence; }

	private:
		static std::vector<VkSemaphore> s_RenderSemaphores;
		static bool s_RenderSemaphoresInitialized;

		VkCommandPool		m_Pool{ VK_NULL_HANDLE };
		VkCommandBuffer		m_CmdBuffer{ VK_NULL_HANDLE };
		VkSemaphore			m_ImageAcquireSemaphore{ VK_NULL_HANDLE };
		VkFence				m_RenderFence{ VK_NULL_HANDLE };
	};

}