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

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		vkb::SwapchainBuilder swapchainBuilder{ physicalDevice, device, surface };
		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.set_desired_format(VkSurfaceFormatKHR{ .format = format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(window->GetWidth(), window->GetHeight())
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

		m_Swapchain = vkbSwapchain.swapchain;

		// ========================================================
		// Images
		// ========================================================
		auto extent				= vkbSwapchain.extent;
		auto swapchainImages	= vkbSwapchain.get_images().value();
		auto views				= vkbSwapchain.get_image_views().value();

		m_Images.reserve(swapchainImages.size());
		for (int i = 0; i < swapchainImages.size(); ++i)
		{
			m_Images.push_back(std::make_shared<VulkanImage>(
				swapchainImages[i], 
				views[i], 
				format, 
				VkExtent3D{.width = extent.width, .height = extent.height, .depth = 1})
			);
		}
	}

}