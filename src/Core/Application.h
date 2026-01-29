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

		static Application*				 GetRaw()			  { return s_AppInstance; }
		std::shared_ptr<Window>			 GetWindow()	const { return m_Window; }
		std::shared_ptr<VulkanRenderer>  GetRenderer()	const { return m_Renderer; }

	private:
		static Application*					s_AppInstance;
		std::shared_ptr<Window>				m_Window;
		std::shared_ptr<VulkanRenderer>     m_Renderer;
	};
}