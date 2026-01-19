#include "VulkanCore.h"
#include "VulkanRenderer.h"
#include "LifetimeManager.h"
#include "VulkanUtils.h"
#include "VkBootstrap.h"
#include <GLFW/glfw3.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace tiny_vulkan {

	VulkanCore::VulkanCore()
	{
		// ========================================================
		// VkInstance + VkDebugUtilsMessengerEXT
		// ========================================================
		vkb::InstanceBuilder instanceBuilder;

		auto vkbInstance = instanceBuilder
			.set_app_name("Tiny Vulkan")
			.request_validation_layers(true)
			.use_default_debug_messenger()
			.require_api_version(1, 4, 0)
			.build()
			.value();

		m_Instance = vkbInstance.instance;
		m_DebugMessenger = vkbInstance.debug_messenger;

		// ========================================================
		// VkSurfaceKHR
		// ========================================================
		auto window = VulkanRenderer::GetWindow();
		glfwCreateWindowSurface(m_Instance, window->GetRaw(), nullptr, &m_Surface);

		// ========================================================
		// VkPhysicalDevice
		// ========================================================
		// vulkan 1.3 features
		VkPhysicalDeviceVulkan13Features features13 = {};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.dynamicRendering = true; // for render pass skip
		features13.synchronization2 = true; // for new sync

		// vulkan 1.2 features
		VkPhysicalDeviceVulkan12Features features12 = {};
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.bufferDeviceAddress = true; // pointers to buffers
		features12.descriptorIndexing = true; // unlocks bindless ()

		// physical device
		vkb::PhysicalDeviceSelector selector{ vkbInstance };
		vkb::PhysicalDevice vkbPhysicalDevice = selector
			.set_minimum_version(1, 3)
			.set_required_features_13(features13)
			.set_required_features_12(features12)
			.set_surface(m_Surface)
			.select()
			.value();

		m_PhysicalDevice = vkbPhysicalDevice.physical_device;

		// ========================================================
		// VkDevice
		// ========================================================
		vkb::DeviceBuilder deviceBuilder{ vkbPhysicalDevice };
		vkb::Device vkbDevice = deviceBuilder
			.build()
			.value();

		m_Device = vkbDevice.device;

		m_GraphicsFamilyIndex	= vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
		m_PresentFamilyIndex	= vkbDevice.get_queue_index(vkb::QueueType::present).value();
		m_GraphicsQueue			= vkbDevice.get_queue(vkb::QueueType::graphics).value();
		m_PresentQueue			= vkbDevice.get_queue(vkb::QueueType::present).value();

		// ========================================================
		// VulkanSwapchain
		// ========================================================
		m_Swapchain = std::make_shared<VulkanSwapchain>(m_PhysicalDevice, m_Device, m_Surface);

		// ========================================================
		// VmaAllocator
		// ========================================================
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice	= m_PhysicalDevice;
		allocatorInfo.device			= m_Device;
		allocatorInfo.instance			= m_Instance;
		allocatorInfo.flags				= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		CHECK_VK_RES(vmaCreateAllocator(&allocatorInfo, &m_Allocator));

		// ========================================================
		// Render target
		// ========================================================
		auto swapchainImage = m_Swapchain->GetImages()[0];
		VkFormat renderTargetFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		VkExtent3D swapchainImageExtent = swapchainImage->GetExtent();

		// Render target info
		VkImageCreateInfo renderTargetInfo = {};
		renderTargetInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		renderTargetInfo.pNext = nullptr;
		renderTargetInfo.flags = 0;
		renderTargetInfo.imageType = VK_IMAGE_TYPE_2D;
		renderTargetInfo.format = renderTargetFormat;
		renderTargetInfo.extent = swapchainImageExtent;
		renderTargetInfo.mipLevels = 1;
		renderTargetInfo.arrayLayers = 1;
		renderTargetInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		renderTargetInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // texels are laid out in such a way how GPU wants
		renderTargetInfo.usage = 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
			VK_IMAGE_USAGE_STORAGE_BIT		| 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		renderTargetInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only 1 family will access it
		renderTargetInfo.queueFamilyIndexCount = 0;
		renderTargetInfo.pQueueFamilyIndices = nullptr;
		renderTargetInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// Allocation info (on GPU)
		VmaAllocationCreateInfo allocationInfo = {};
		allocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		allocationInfo.priority = 1.0f;

		VkImage renderTargetImage{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		vmaCreateImage(m_Allocator, &renderTargetInfo, &allocationInfo, &renderTargetImage, &allocation, nullptr);

		// VkImageView
		VkImageViewCreateInfo renderTargetViewInfo = {};
		renderTargetViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		renderTargetViewInfo.pNext = nullptr;
		renderTargetViewInfo.flags = 0;
		renderTargetViewInfo.image = renderTargetImage;
		renderTargetViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		renderTargetViewInfo.format = renderTargetFormat;
		renderTargetViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		renderTargetViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		renderTargetViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		renderTargetViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		renderTargetViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		renderTargetViewInfo.subresourceRange.baseArrayLayer = 0;
		renderTargetViewInfo.subresourceRange.baseMipLevel = 0;
		renderTargetViewInfo.subresourceRange.layerCount = 1;
		renderTargetViewInfo.subresourceRange.levelCount = 1;

		VkImageView renderTargetView{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateImageView(m_Device, &renderTargetViewInfo, nullptr, &renderTargetView));

		// Render target
		m_RenderTarget = std::make_shared<VulkanImage>(renderTargetImage, renderTargetView, renderTargetFormat, swapchainImageExtent, allocation);

		// ========================================================
		// Register cleanup
		// ========================================================
		LifetimeManager::PushFunction(vkDestroyInstance, m_Instance, nullptr);
		LifetimeManager::PushFunction(vkb::destroy_debug_utils_messenger, m_Instance, m_DebugMessenger, nullptr);
		LifetimeManager::PushFunction(vkDestroyDevice, m_Device, nullptr);
		LifetimeManager::PushFunction(vkDestroySurfaceKHR, m_Instance, m_Surface, nullptr);

		auto& images = m_Swapchain->GetImages();
		LifetimeManager::PushFunction(vkDestroySwapchainKHR, m_Device, m_Swapchain->GetRaw(), nullptr);
		for (int i = 0; i < images.size(); ++i)
		{
			LifetimeManager::PushFunction(vkDestroyImageView, m_Device, images[i]->GetView(), nullptr);
		}

		LifetimeManager::PushFunction(vmaDestroyAllocator, m_Allocator);
		LifetimeManager::PushFunction(vmaDestroyImage, m_Allocator, renderTargetImage, allocation);
		LifetimeManager::PushFunction(vkDestroyImageView, m_Device, renderTargetView, nullptr);
	}

}