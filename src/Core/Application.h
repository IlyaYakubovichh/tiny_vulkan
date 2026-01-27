#pragma once

#include "Window.h"
#include "VulkanRenderer.h"
#include <memory>

namespace tiny_vulkan {

	struct ApplicationSpec
	{
		uint32_t windowWidth;
		uint32_t windowHeight;
		const char* windowName;
	};

	class Application 
	{
	public:
		Application(const ApplicationSpec& appSpec);
		~Application();

		void Run();

	private:
		std::shared_ptr<Window>				m_Window;
		std::shared_ptr<VulkanRenderer>     m_Renderer;
	};
}