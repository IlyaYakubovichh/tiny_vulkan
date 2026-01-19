#pragma once

#include "Window.h"
#include "VulkanCore.h"
#include "VulkanFrame.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <array>

namespace tiny_vulkan {

	constexpr uint32_t FRAMES_IN_FLIGHT = 3;

	class VulkanRenderer
	{
	public:
		VulkanRenderer() = delete;
		~VulkanRenderer() = delete;

		static void Initialize();
		static void Shutdown();
		static void Run();

		[[nodiscard]] static auto GetWindow()	{ return s_Window; }
		[[nodiscard]] static auto GetCore()		{ return s_VulkanCore; }

	private:
		static void BeginFrame();
		static void EndFrame();
		static void OnUpdate();
		static void Clear(VkCommandBuffer cmd, glm::vec3 color);

	private:
		static std::shared_ptr<Window>     s_Window;
		static std::shared_ptr<VulkanCore> s_VulkanCore;

		static std::vector<std::shared_ptr<VulkanFrame>> s_Frames;

		static uint32_t s_CurrentFrameIndex;
		static uint32_t s_CurrentImageIndex;
	};

}