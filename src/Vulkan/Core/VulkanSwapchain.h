#pragma once

#include "VulkanImage.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace tiny_vulkan {

	class VulkanSwapchain
	{
	public:
		explicit VulkanSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height);
		~VulkanSwapchain() = default;

		VulkanSwapchain(const VulkanSwapchain&) = delete;
		VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

		[[nodiscard]] VkSwapchainKHR	GetRaw()	const { return m_Swapchain; }
		[[nodiscard]] VkFormat			GetFormat() const { return m_Format; }
		[[nodiscard]] VkExtent2D		GetExtent() const { return m_Extent; }
		[[nodiscard]] const std::vector<std::shared_ptr<VulkanImage>>& GetImages() const { return m_Images; }

	private:
		VkSwapchainKHR		m_Swapchain{ VK_NULL_HANDLE };
		VkFormat			m_Format{ VK_FORMAT_UNDEFINED };
		VkExtent2D			m_Extent{ 0, 0 };
		std::vector<std::shared_ptr<VulkanImage>> m_Images;
	};

}