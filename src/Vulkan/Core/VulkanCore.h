#pragma once

#include "Window.h"
#include "VulkanFrame.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "VkBootstrap.h"

#include <memory>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace tiny_vulkan {

	class VulkanCore
	{
	public:
		VulkanCore() = delete;
		VulkanCore(const VulkanCore&) = delete;
		VulkanCore& operator=(const VulkanCore&) = delete;

		static void Initialize(std::shared_ptr<Window> window);
		static void AdvanceFrame();

		[[nodiscard]] static VulkanCore*								 GetRaw() { return s_CoreInstance; }
		[[nodiscard]] static VkInstance									 GetInstance() { return s_Instance; }
		[[nodiscard]] static VkPhysicalDevice							 GetPhysicalDevice() { return s_PhysicalDevice; }
		[[nodiscard]] static VkDevice									 GetDevice() { return s_Device; }
		[[nodiscard]] static VkSurfaceKHR								 GetSurface() { return s_Surface; }
		[[nodiscard]] static std::shared_ptr<VulkanSwapchain>			 GetSwapchain() { return s_Swapchain; }
		[[nodiscard]] static std::shared_ptr<VulkanImage>				 GetRenderTarget() { return s_RenderTarget; }
		[[nodiscard]] static std::shared_ptr<VulkanImage>				 GetDepthImage() { return s_DepthImage; }
		[[nodiscard]] static VkQueue									 GetGraphicsQueue() { return s_GraphicsQueue; }
		[[nodiscard]] static VkQueue									 GetPresentQueue() { return s_PresentQueue; }
		[[nodiscard]] static uint32_t									 GetGraphicsFamily() { return s_GraphicsFamilyIndex; }
		[[nodiscard]] static VmaAllocator								 GetVmaAllocator() { return s_Allocator; }
		[[nodiscard]] static std::vector<std::shared_ptr<VulkanFrame>>&  GetFrames() { return s_Frames; }
		[[nodiscard]] static std::shared_ptr<VulkanFrame>&				 GetCurrentFrame() { return s_Frames[s_CurrentFrameIndex]; }

	private:
		static void CreateInstance();
		static void CreateSurface(GLFWwindow* window);
		static void SelectPhysicalDevice();
		static void CreateLogicalDevice();
		static void CreateAllocator();
		static void CreateSwapchain();
		static void CreateRenderTarget();
		static void CreateDepthImage();
		static void CreateFrames();

	private:
		static VulkanCore*									s_CoreInstance;
		static std::shared_ptr<Window>						s_Window;
		static vkb::Instance								s_VkbInstance;
		static vkb::PhysicalDevice							s_VkbPhysicalDevice;
		static VkInstance									s_Instance;
		static VkDebugUtilsMessengerEXT						s_DebugMessenger;
		static VkSurfaceKHR									s_Surface;
		static VkPhysicalDevice								s_PhysicalDevice;
		static VkDevice										s_Device;
		static VmaAllocator									s_Allocator;
		static std::shared_ptr<VulkanSwapchain>				s_Swapchain;
		static std::shared_ptr<VulkanImage>					s_RenderTarget;
		static std::shared_ptr<VulkanImage>					s_DepthImage;
		static uint32_t										s_GraphicsFamilyIndex;
		static uint32_t										s_PresentFamilyIndex;
		static VkQueue										s_GraphicsQueue;
		static VkQueue										s_PresentQueue;
		static std::vector<std::shared_ptr<VulkanFrame>>	s_Frames;
		static uint32_t										s_FlightFrameCount;
		static uint32_t										s_CurrentFrameIndex;
	};

}