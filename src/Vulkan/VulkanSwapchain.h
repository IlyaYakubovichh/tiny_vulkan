#pragma once

#include "VulkanImage.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace tiny_vulkan {

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);
		~VulkanSwapchain() = default;

		[[nodiscard]] auto	GetRaw()	 const { return m_Swapchain; }
		[[nodiscard]] auto&	GetImages()	 const { return m_Images; }

	private:
		VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };

		std::vector<std::shared_ptr<VulkanImage>> m_Images;
	};

}