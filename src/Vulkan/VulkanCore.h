#pragma once

#include "VulkanSwapchain.h"
#include <vulkan/vulkan.h>
#include <memory>

namespace tiny_vulkan {

	class VulkanCore
	{
	public:
		VulkanCore();
		~VulkanCore() = default;

		auto GetSwapchain() const { return m_Swapchain; }
		auto GetDevice() const { return m_Device; }
		auto GetGraphicsQueue() const { return m_GraphicsQueue; }
		auto GetPresentQueue() const { return m_PresentQueue; }
		auto GetGraphicsFamily() const { return m_GraphicsFamilyIndex; }
		
	private:
		VkInstance							m_Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT			m_DebugMessenger{ VK_NULL_HANDLE };
		VkPhysicalDevice					m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice							m_Device{ VK_NULL_HANDLE };
		VkSurfaceKHR						m_Surface{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanSwapchain>	m_Swapchain;

		uint32_t m_GraphicsFamilyIndex;	VkQueue m_GraphicsQueue;
		uint32_t m_PresentFamilyIndex;	VkQueue m_PresentQueue;
	};

}