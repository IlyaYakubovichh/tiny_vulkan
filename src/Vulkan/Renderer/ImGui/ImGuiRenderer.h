#pragma once

#include "Window.h"
#include "VulkanImage.h"

#include <memory>
#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer(std::shared_ptr<Window> window);
		~ImGuiRenderer() = default;

		void DrawImGui(std::shared_ptr<VulkanImage> swapchainImage);

	private:
		void Render();

	private:
		VkDescriptorPool			 m_Pool{ VK_NULL_HANDLE };
		std::shared_ptr<Window>		 m_Window;
	};

}