#include "VulkanCore.h"
#include "VulkanRenderer.h"
#include "LifetimeManager.h"
#include "VkBootstrap.h"
#include <GLFW/glfw3.h>

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
	}

}