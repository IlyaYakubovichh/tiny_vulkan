#include "VulkanCore.h"
#include "Application.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

#ifndef VMA_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif

#include <GLFW/glfw3.h>

namespace tiny_vulkan {

	VulkanCore*						 VulkanCore::s_CoreInstance = nullptr;
	std::shared_ptr<Window>			 VulkanCore::s_Window = nullptr;
	vkb::Instance					 VulkanCore::s_VkbInstance = {};
	vkb::PhysicalDevice				 VulkanCore::s_VkbPhysicalDevice = {};
	VkInstance                       VulkanCore::s_Instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT         VulkanCore::s_DebugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR                     VulkanCore::s_Surface = VK_NULL_HANDLE;
	VkPhysicalDevice                 VulkanCore::s_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice                         VulkanCore::s_Device = VK_NULL_HANDLE;
	VmaAllocator                     VulkanCore::s_Allocator = VK_NULL_HANDLE;
	std::shared_ptr<VulkanSwapchain> VulkanCore::s_Swapchain = nullptr;
	std::shared_ptr<VulkanImage>     VulkanCore::s_RenderTarget = nullptr;
	std::shared_ptr<VulkanImage>	 VulkanCore::s_DepthImage = nullptr;
	uint32_t						 VulkanCore::s_GraphicsFamilyIndex = 0;
	uint32_t						 VulkanCore::s_PresentFamilyIndex = 0;
	VkQueue							 VulkanCore::s_GraphicsQueue = VK_NULL_HANDLE;
	VkQueue							 VulkanCore::s_PresentQueue = VK_NULL_HANDLE;

	std::vector<std::shared_ptr<tiny_vulkan::VulkanFrame>> VulkanCore::s_Frames;
	uint32_t VulkanCore::s_FlightFrameCount = 3;
	uint32_t VulkanCore::s_CurrentFrameIndex = 0;

	void VulkanCore::Initialize(std::shared_ptr<Window> window)
	{
		s_Window = window;

		CreateInstance();
		CreateSurface(s_Window->GetRaw());
		SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateAllocator();
		CreateSwapchain();
		CreateRenderTarget();
		CreateDepthImage();
		CreateFrames();
	}

	void VulkanCore::AdvanceFrame()
	{
		s_CurrentFrameIndex = (s_CurrentFrameIndex + 1) % s_FlightFrameCount;
	}

	void VulkanCore::CreateInstance()
	{
		vkb::InstanceBuilder instanceBuilder;
		s_VkbInstance = instanceBuilder
			.set_app_name("Tiny Vulkan")
			.request_validation_layers(false)
			.use_default_debug_messenger()
			.require_api_version(1, 4, 0)
			.build()
			.value();

		s_Instance = s_VkbInstance.instance;
		s_DebugMessenger = s_VkbInstance.debug_messenger;

		LifetimeManager::PushFunction(vkDestroyInstance, s_Instance, nullptr);
		LifetimeManager::PushFunction(vkb::destroy_debug_utils_messenger, s_Instance, s_DebugMessenger, nullptr);
	}

	void VulkanCore::CreateSurface(GLFWwindow* window)
	{
		CHECK_VK_RES(glfwCreateWindowSurface(s_Instance, window, nullptr, &s_Surface));
		LifetimeManager::PushFunction(vkDestroySurfaceKHR, s_Instance, s_Surface, nullptr);
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

		vkb::PhysicalDeviceSelector selector{ s_VkbInstance };
		s_VkbPhysicalDevice = selector
			.set_minimum_version(1, 4)
			.set_required_features_13(features13)
			.set_required_features_12(features12)
			.set_surface(s_Surface)
			.select()
			.value();

		s_PhysicalDevice = s_VkbPhysicalDevice.physical_device;

		VkPhysicalDeviceProperties physicalDeviceProps;
		vkGetPhysicalDeviceProperties(s_PhysicalDevice, &physicalDeviceProps);
		LOG_INFO(fmt::runtime("Selected GPU info: \n\t->Device name: {0} \n\t->ApiVersion: {1} \n\t->Driver version: {2}"),
			physicalDeviceProps.deviceName,
			physicalDeviceProps.apiVersion,
			physicalDeviceProps.driverVersion
		);
	}

	void VulkanCore::CreateLogicalDevice()
	{
		vkb::DeviceBuilder deviceBuilder{ s_VkbPhysicalDevice };
		vkb::Device vkbDevice = deviceBuilder.build().value();

		s_Device = vkbDevice.device;
		s_GraphicsFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
		s_PresentFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::present).value();
		s_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		s_PresentQueue = vkbDevice.get_queue(vkb::QueueType::present).value();

		LifetimeManager::PushFunction(vkDestroyDevice, s_Device, nullptr);
	}

	void VulkanCore::CreateAllocator()
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = s_PhysicalDevice;
		allocatorInfo.device = s_Device;
		allocatorInfo.instance = s_Instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		CHECK_VK_RES(vmaCreateAllocator(&allocatorInfo, &s_Allocator));
		LifetimeManager::PushFunction(vmaDestroyAllocator, s_Allocator);
	}

	void VulkanCore::CreateSwapchain()
	{
		s_Swapchain = std::make_shared<VulkanSwapchain>(
			s_Window->GetWidth(),
			s_Window->GetHeight()
		);
	}

	void VulkanCore::CreateRenderTarget()
	{
		// Settings
		const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
		const VkExtent3D extent = s_Swapchain->GetImages()[0]->GetExtent();

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
			VK_IMAGE_USAGE_STORAGE_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Allocation Info
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocInfo.priority = 1.0f;

		// Create image
		VkImage image{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		CHECK_VK_RES(vmaCreateImage(s_Allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr));

		// View Info
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.baseMipLevel = 0;

		VkImageView view{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateImageView(s_Device, &viewInfo, nullptr, &view));

		// Store result
		s_RenderTarget = std::make_shared<VulkanImage>(image, view, format, extent, allocation);

		// Cleanup
		LifetimeManager::PushFunction(vmaDestroyImage, s_Allocator, image, allocation);
		LifetimeManager::PushFunction(vkDestroyImageView, s_Device, view, nullptr);
	}

	void VulkanCore::CreateDepthImage()
	{
		VkExtent3D depthExtent = s_RenderTarget->GetExtent();
		VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

		VkImageCreateInfo depthInfo = {};
		depthInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		depthInfo.pNext = nullptr;
		depthInfo.flags = 0;
		depthInfo.imageType = VK_IMAGE_TYPE_2D;
		depthInfo.format = depthFormat;
		depthInfo.extent = depthExtent;
		depthInfo.mipLevels = 1;
		depthInfo.arrayLayers = 1;
		depthInfo.samples = VK_SAMPLE_COUNT_1_BIT;	
		depthInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		depthInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		depthInfo.queueFamilyIndexCount = 0;
		depthInfo.pQueueFamilyIndices = nullptr;
		depthInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocationInfo = {};
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocationInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkImage depthImage{ VK_NULL_HANDLE };
		VkImageView depthView{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		vmaCreateImage(s_Allocator, &depthInfo, &allocationInfo, &depthImage, &allocation, nullptr);

		VkImageViewCreateInfo depthViewInfo = {};
		depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthViewInfo.pNext = nullptr;
		depthViewInfo.flags = 0;
		depthViewInfo.image = depthImage;
		depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthViewInfo.format = depthFormat;
		depthViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		depthViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		depthViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		depthViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthViewInfo.subresourceRange.baseArrayLayer = 0;
		depthViewInfo.subresourceRange.baseMipLevel = 0;
		depthViewInfo.subresourceRange.layerCount = 1;
		depthViewInfo.subresourceRange.levelCount = 1;
		CHECK_VK_RES(vkCreateImageView(s_Device, &depthViewInfo, nullptr, &depthView));

		s_DepthImage = std::make_shared<VulkanImage>(depthImage, depthView, depthFormat, depthExtent, allocation);

		LifetimeManager::PushFunction(vmaDestroyImage, s_Allocator, depthImage, allocation);
		LifetimeManager::PushFunction(vkDestroyImageView, s_Device, depthView, nullptr);
	}

	void VulkanCore::CreateFrames()
	{
		s_Frames.reserve(s_FlightFrameCount);
		for (uint32_t i = 0; i < s_FlightFrameCount; ++i)
		{
			s_Frames.push_back(std::make_shared<VulkanFrame>());
		}
	}

}