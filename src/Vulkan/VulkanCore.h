#pragma once

#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>

namespace tiny_vulkan {

	class VulkanCore
	{
	public:
		VulkanCore();
		~VulkanCore() = default;

		[[nodiscard]] auto GetSwapchain()		const { return m_Swapchain; }
		[[nodiscard]] auto GetDevice()			const { return m_Device; }
		[[nodiscard]] auto GetGraphicsQueue()	const { return m_GraphicsQueue; }
		[[nodiscard]] auto GetPresentQueue()	const { return m_PresentQueue; }
		[[nodiscard]] auto GetGraphicsFamily()	const { return m_GraphicsFamilyIndex; }
		[[nodiscard]] auto GetVmaAllocator()	const { return m_Allocator; }
		[[nodiscard]] auto GetRenderTarget()	const { return m_RenderTarget; }
		
	private:
		VkInstance							m_Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT			m_DebugMessenger{ VK_NULL_HANDLE };
		VkPhysicalDevice					m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice							m_Device{ VK_NULL_HANDLE };
		VkSurfaceKHR						m_Surface{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanSwapchain>	m_Swapchain;
		VmaAllocator						m_Allocator{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanImage>		m_RenderTarget;

		uint32_t m_GraphicsFamilyIndex;	VkQueue m_GraphicsQueue;
		uint32_t m_PresentFamilyIndex;	VkQueue m_PresentQueue;
	};

}