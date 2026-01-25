#pragma once

#include "Window.h"
#include "VulkanCore.h"
#include "ImGui/ImGuiRenderer.h"
#include "VulkanFrame.h"
#include <memory>
#include <vector>

namespace tiny_vulkan {

	class Scene;

	class VulkanRenderer
	{
	public:
		explicit VulkanRenderer(std::shared_ptr<Window> window, std::shared_ptr<VulkanCore> core);
		~VulkanRenderer() = default;

		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(const VulkanRenderer&) = delete;

		void Draw(Scene& scene);

		[[nodiscard]] uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

	private:
		void BeginFrame();
		void EndFrame();

	private:
		std::shared_ptr<ImGuiRenderer>				m_ImGuiRenderer;
		std::shared_ptr<VulkanCore>					m_Core;
		std::shared_ptr<Window>						m_Window;
		std::vector<std::shared_ptr<VulkanFrame>>	m_Frames;
		uint32_t									m_CurrentFrameIndex{ 0 };
		uint32_t									m_CurrentImageIndex{ 0 };
		const uint32_t								m_FlightFrameCount{ 3 };
	};

}