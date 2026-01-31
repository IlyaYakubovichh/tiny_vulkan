#include "VulkanSwapchain.h"
#include "VulkanCore.h"
#include "LifetimeManager.h"
#include "VkBootstrap.h"

namespace tiny_vulkan {

	VulkanSwapchain::VulkanSwapchain(uint32_t width, uint32_t height)
	{
		CreateSwapchain(width, height);
	}

	void VulkanSwapchain::CreateSwapchain(uint32_t width, uint32_t height)
	{
		auto device = VulkanCore::GetDevice();
		auto physicalDevice = VulkanCore::GetPhysicalDevice();
		auto surface = VulkanCore::GetSurface();

		// ========================================================
		// Configuration
		// ========================================================
		const VkFormat desiredFormat = VK_FORMAT_R8G8B8A8_UNORM;
		const VkColorSpaceKHR desiredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		const VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR; // V-Sync enabled by default

		// ========================================================
		// Builder
		// ========================================================
		vkb::SwapchainBuilder swapchainBuilder{ physicalDevice, device, surface };

		auto vkbSwapchainResult = swapchainBuilder
			.set_desired_format(VkSurfaceFormatKHR{ .format = desiredFormat, .colorSpace = desiredColorSpace })
			.set_desired_present_mode(desiredPresentMode)
			.set_desired_extent(width, height)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build();

		vkb::Swapchain vkbSwapchain = vkbSwapchainResult.value();
		m_Swapchain = vkbSwapchain.swapchain;
		m_Format = vkbSwapchain.image_format;
		m_Extent = vkbSwapchain.extent;

		// ========================================================
		// Wrap Images
		// ========================================================
		// vkb::Swapchain helper returns std::vector<VkImage> and std::vector<VkImageView>
		auto images = vkbSwapchain.get_images().value();
		auto views = vkbSwapchain.get_image_views().value();

		m_Images.reserve(images.size());

		for (size_t i = 0; i < images.size(); ++i)
		{
			m_Images.push_back(std::make_shared<VulkanImage>(
				images[i],
				views[i],
				m_Format,
				VkExtent3D{ .width = m_Extent.width, .height = m_Extent.height, .depth = 1 },
				VmaAllocation{ VK_NULL_HANDLE }
			));
		}

		// Register cleanup
		for (auto view : views)
		{
			m_OwnDeleters.push_back([=]() -> void
				{
					vkDestroyImageView(device, view, nullptr);
				}
			);
		}
		m_OwnDeleters.push_back([=]() -> void
			{
				vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
			}
		);
	}

	void VulkanSwapchain::RecreateSwapchain(uint32_t width, uint32_t height)
	{
		auto device = VulkanCore::GetDevice();
		CHECK_VK_RES(vkDeviceWaitIdle(VulkanCore::GetDevice()));

		CleanupResources();

		CreateSwapchain(width, height);
	}

	void VulkanSwapchain::CleanupResources()
	{
		for (auto it = m_OwnDeleters.rbegin(); it != m_OwnDeleters.rend(); ++it) 
		{
			if (*it)
			{
				(*it)();
			}
		}
		m_OwnDeleters.clear();
		m_Images.clear();
		m_Swapchain = VK_NULL_HANDLE;
	}

}