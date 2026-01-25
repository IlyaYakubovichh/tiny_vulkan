#include "VulkanCore.h"
#include "LifetimeManager.h"

#ifndef VMA_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif

#include <GLFW/glfw3.h>

namespace tiny_vulkan {

	VulkanCore::VulkanCore(std::shared_ptr<Window> window)
		: m_Window(window)
	{
		CreateInstance();
		CreateSurface(window->GetRaw());
		SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateAllocator();
		CreateSwapchain();
		CreateRenderTarget();
	}

	void VulkanCore::CreateInstance()
	{
		vkb::InstanceBuilder instanceBuilder;
		m_VkbInstance = instanceBuilder
			.set_app_name("Tiny Vulkan")
			.request_validation_layers(true)
			.use_default_debug_messenger()
			.require_api_version(1, 4, 0)
			.build()
			.value();

		m_Instance = m_VkbInstance.instance;
		m_DebugMessenger = m_VkbInstance.debug_messenger;

		LifetimeManager::PushFunction(vkDestroyInstance, m_Instance, nullptr);
		LifetimeManager::PushFunction(vkb::destroy_debug_utils_messenger, m_Instance, m_DebugMessenger, nullptr);
	}

	void VulkanCore::CreateSurface(GLFWwindow* window)
	{
		CHECK_VK_RES(glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface));
		LifetimeManager::PushFunction(vkDestroySurfaceKHR, m_Instance, m_Surface, nullptr);
	}

	void VulkanCore::SelectPhysicalDevice()
	{
		VkPhysicalDeviceVulkan13Features features13 = {};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.dynamicRendering = true;
		features13.synchronization2 = true;

		VkPhysicalDeviceVulkan12Features features12 = {};
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.bufferDeviceAddress = true;
		features12.descriptorIndexing = true;

		vkb::PhysicalDeviceSelector selector{ m_VkbInstance }; 
		m_VkbPhysicalDevice = selector
			.set_minimum_version(1, 4)
			.set_required_features_13(features13)
			.set_required_features_12(features12)
			.set_surface(m_Surface)
			.select()
			.value();

		m_PhysicalDevice = m_VkbPhysicalDevice.physical_device;
	}

	void VulkanCore::CreateLogicalDevice()
	{
		vkb::DeviceBuilder deviceBuilder{ m_VkbPhysicalDevice };
		vkb::Device vkbDevice = deviceBuilder.build().value();

		m_Device = vkbDevice.device;
		m_GraphicsFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
		m_PresentFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::present).value();
		m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		m_PresentQueue = vkbDevice.get_queue(vkb::QueueType::present).value();

		LifetimeManager::PushFunction(vkDestroyDevice, m_Device, nullptr);
	}

	void VulkanCore::CreateAllocator()
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = m_PhysicalDevice;
		allocatorInfo.device = m_Device;
		allocatorInfo.instance = m_Instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		CHECK_VK_RES(vmaCreateAllocator(&allocatorInfo, &m_Allocator));
		LifetimeManager::PushFunction(vmaDestroyAllocator, m_Allocator);
	}

	void VulkanCore::CreateSwapchain()
	{
		m_Swapchain = std::make_shared<VulkanSwapchain>(
			m_PhysicalDevice, 
			m_Device, m_Surface, 
			m_Window->GetWidth(),
			m_Window->GetHeight()
		);

		LifetimeManager::PushFunction(vkDestroySwapchainKHR, m_Device, m_Swapchain->GetRaw(), nullptr);

		for (const auto& image : m_Swapchain->GetImages())
		{
			LifetimeManager::PushFunction(vkDestroyImageView, m_Device, image->GetView(), nullptr);
		}
	}

	void VulkanCore::CreateRenderTarget()
	{
		// Settings
		const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
		const VkExtent3D extent = m_Swapchain->GetImages()[0]->GetExtent();

		// Image Info
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.extent = extent;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_STORAGE_BIT		| 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Allocation Info
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocInfo.priority = 1.0f;

		VkImage image{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		CHECK_VK_RES(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr));

		// View Info
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView view{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateImageView(m_Device, &viewInfo, nullptr, &view));

		// Store result
		m_RenderTarget = std::make_shared<VulkanImage>(image, view, format, extent, allocation);

		// Cleanup
		LifetimeManager::PushFunction(vmaDestroyImage, m_Allocator, image, allocation);
		LifetimeManager::PushFunction(vkDestroyImageView, m_Device, view, nullptr);
	}
}