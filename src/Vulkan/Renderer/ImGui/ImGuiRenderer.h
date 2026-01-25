#pragma once

#include "Window.h"
#include "VulkanCore.h"

#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	struct ImGuiOnUpdatePackage
	{
		VkCommandBuffer cmdBuffer;
		VkImageView		imageView;
		VkImageLayout	imageLayout;
		VkExtent3D		extent;
	};

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer(std::shared_ptr<Window> window, std::shared_ptr<VulkanCore> core);
		~ImGuiRenderer() = default;

		void OnUpdate(const ImGuiOnUpdatePackage& package);

	private:
		void CalculateInternal();

	private:
		VkDescriptorPool			 m_Pool{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanCore>  m_Core;
		std::shared_ptr<Window>		 m_Window;
	};

}