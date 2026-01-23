#pragma once

#include "Window.h"
#include "VulkanCore.h"
#include "VulkanFrame.h"
#include "RendererData.h"
#include <glm/glm.hpp>
#include <memory>
#include <span>
#include <vector>

namespace tiny_vulkan {

	constexpr uint32_t FRAMES_IN_FLIGHT = 3;

	class VulkanRenderer
	{
	public:
		VulkanRenderer() = delete;
		~VulkanRenderer() = delete;

		static void Run();
		static void ImmediateSubmit(std::function<void(VkCommandBuffer cmdBuffer)>&& func);

		[[nodiscard]] static auto&	GetRendererData()	{ return s_Data; }
		[[nodiscard]] static auto	GetWindow()			{ return s_Window; }
		[[nodiscard]] static auto	GetCore()			{ return s_VulkanCore; }

	private:
		static void Initialize();
		static void Shutdown();
		static void BeginFrame();
		static void EndFrame();
		static void OnUpdate();
		static void Clear(VkCommandBuffer cmd, glm::vec3 color);
		static void Dispatch(VkCommandBuffer cmdBuffer, uint32_t groupX, uint32_t groupY, uint32_t groupZ);
		static void DrawRectangle(VkCommandBuffer cmdBuffer);

	private:
		static std::shared_ptr<RendererData>	s_Data;
		static std::shared_ptr<Window>			s_Window;
		static std::shared_ptr<VulkanCore>		s_VulkanCore;

		static std::vector<std::shared_ptr<VulkanFrame>> s_Frames;

		static uint32_t s_CurrentFrameIndex;
		static uint32_t s_CurrentImageIndex;
	};

}