#include "VulkanSwapchain.h"
#include "VulkanRenderer.h"
#include "LifetimeManager.h"
#include "VkBootstrap.h"

namespace tiny_vulkan {

	VulkanSwapchain::VulkanSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface)
	{
		// ========================================================
		// Core
		// ========================================================
		auto window = VulkanRenderer::GetWindow();
		m_SwapchainImageFormat = VK_FORMAT_R8G8B8A8_UNORM;

		vkb::SwapchainBuilder swapchainBuilder{ physicalDevice, device, surface };
		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.set_desired_format(VkSurfaceFormatKHR{ .format = m_SwapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(window->GetWidth(), window->GetHeight())
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

		m_SwapchainExtent		= vkbSwapchain.extent;
		m_Swapchain				= vkbSwapchain.swapchain;
		m_SwapchainImages		= vkbSwapchain.get_images().value();
		m_SwapchainImageViews	= vkbSwapchain.get_image_views().value();
	}

}