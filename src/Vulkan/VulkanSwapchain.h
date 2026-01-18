#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace tiny_vulkan {

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);
		~VulkanSwapchain() = default;

		auto	GetRaw()			const { return m_Swapchain; }
		auto	GetImageFormat()	const { return m_SwapchainImageFormat; }
		auto	GetExtent()			const { return m_SwapchainExtent; }
		auto&	GetImages()			const { return m_SwapchainImages; }
		auto&	GetViews()			const { return m_SwapchainImageViews; }

	private:
		VkSwapchainKHR				m_Swapchain{ VK_NULL_HANDLE };

		VkFormat					m_SwapchainImageFormat;
		std::vector<VkImage>		m_SwapchainImages;
		std::vector<VkImageView>	m_SwapchainImageViews;
		VkExtent2D					m_SwapchainExtent;
	};

}