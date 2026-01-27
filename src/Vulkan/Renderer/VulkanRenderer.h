#pragma once

#include "Scene.h"
#include "Window.h"
#include "VulkanFrame.h"
#include "ImGui/ImGuiRenderer.h"
#include <memory>
#include <vector>

namespace tiny_vulkan {

	class VulkanRenderer
	{
	public:
		explicit VulkanRenderer(std::shared_ptr<Window> window);
		~VulkanRenderer() = default;

		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(const VulkanRenderer&) = delete;

		void Draw();

	private:
		void BeginFrame();
		void EndFrame();
		void CopyRenderTargetToSwapchain();
		void DrawImGuiToSwapchain();

	private:
		std::shared_ptr<Scene>			m_Scene;
		std::shared_ptr<Window>			m_Window;
		std::shared_ptr<ImGuiRenderer>	m_ImGuiRenderer;
		uint32_t						m_CurrentImageIndex{ 0 };
	};

}