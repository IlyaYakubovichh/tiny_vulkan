#pragma once

#include "Window.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "VkBootstrap.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>

namespace tiny_vulkan {

	class VulkanCore
	{
	public:
		explicit VulkanCore(std::shared_ptr<Window> window);
		~VulkanCore() = default;

		VulkanCore(const VulkanCore&) = delete;
		VulkanCore& operator=(const VulkanCore&) = delete;

		[[nodiscard]] VkInstance            GetInstance()       const { return m_Instance; }
		[[nodiscard]] VkPhysicalDevice      GetPhysicalDevice() const { return m_PhysicalDevice; }
		[[nodiscard]] VkDevice              GetDevice()         const { return m_Device; }
		[[nodiscard]] VulkanSwapchain*		GetSwapchain()      const { return m_Swapchain.get(); }
		[[nodiscard]] VulkanImage*			GetRenderTarget()   const { return m_RenderTarget.get(); }
		[[nodiscard]] VkQueue               GetGraphicsQueue()  const { return m_GraphicsQueue; }
		[[nodiscard]] VkQueue               GetPresentQueue()   const { return m_PresentQueue; }
		[[nodiscard]] uint32_t              GetGraphicsFamily() const { return m_GraphicsFamilyIndex; }
		[[nodiscard]] VmaAllocator          GetVmaAllocator()   const { return m_Allocator; }

	private:
		void CreateInstance();
		void CreateSurface(GLFWwindow* window);
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateAllocator();
		void CreateSwapchain();
		void CreateRenderTarget();

	private:
		std::shared_ptr<Window>			 m_Window;
		vkb::Instance					 m_VkbInstance;
		vkb::PhysicalDevice				 m_VkbPhysicalDevice;
		VkInstance                       m_Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT         m_DebugMessenger{ VK_NULL_HANDLE };
		VkSurfaceKHR                     m_Surface{ VK_NULL_HANDLE };
		VkPhysicalDevice                 m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice                         m_Device{ VK_NULL_HANDLE };
		VmaAllocator                     m_Allocator{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanSwapchain> m_Swapchain;
		std::shared_ptr<VulkanImage>     m_RenderTarget;
		uint32_t						 m_GraphicsFamilyIndex{ 0 };
		uint32_t						 m_PresentFamilyIndex{ 0 };
		VkQueue							 m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue							 m_PresentQueue{ VK_NULL_HANDLE };
	};

}