#pragma once

#include "Window.h"
#include "VulkanCore.h"
#include "VulkanRenderer.h"
#include <memory>

namespace tiny_vulkan {

	class Application 
	{
	public:
		Application();
		~Application();

		void Run();

	private:
		std::shared_ptr<Window>				m_Window;
		std::shared_ptr<VulkanCore>			m_Core;
		std::shared_ptr<VulkanRenderer>     m_Renderer;
	};
}