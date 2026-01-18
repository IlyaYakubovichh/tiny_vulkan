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
		
	private:
		VkInstance							m_Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT			m_DebugMessenger{ VK_NULL_HANDLE };
		VkPhysicalDevice					m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice							m_Device{ VK_NULL_HANDLE };
		VkSurfaceKHR						m_Surface{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanSwapchain>	m_Swapchain;
	};

}