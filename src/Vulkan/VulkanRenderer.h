#pragma once

#include "Window.h"
#include "VulkanCore.h"
#include <memory>

namespace tiny_vulkan {

	class VulkanRenderer
	{
	public:
		VulkanRenderer() = default;
		~VulkanRenderer() = default;

		static auto GetWindow() { return m_Window; }

		static void Initialize();
		static void Shutdown();
		static void Run();

	private:
		static std::shared_ptr<Window>		m_Window;
		static std::shared_ptr<VulkanCore>	m_VulkanCore;
	};

}