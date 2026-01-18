#include "Window.h"
#include "LifetimeManager.h"
#include "LogSystem.h"
#include "VulkanUtils.h"

namespace tiny_vulkan {

	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* description)
	{
		LOG_ERROR(fmt::runtime("GLFW Error ({0}): {1}"), error, description);
	}

	Window::Window()
	{
		// ========================================================
		// Core GLFW
		// ========================================================
		const uint32_t width = 1280;
		const uint32_t height = 720;
		const char* title = "Tiny vulkan";

		LOG_INFO(fmt::runtime("Creating Window: {0} ({1}x{2})"), title, width, height);

		if (!s_GLFWInitialized)
		{
			if (!glfwInit())
			{
				LOG_CRITICAL("Failed to initialize GLFW");
				return;
			}

			s_GLFWInitialized = true;
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); 
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_Window = glfwCreateWindow(
			static_cast<int>(width),
			static_cast<int>(height),
			title,
			nullptr,
			nullptr
		);

		if (!m_Window)
		{
			LOG_CRITICAL("Failed to create GLFW window");
			return;
		}

		// ========================================================
		// Register cleanup
		// ========================================================
		LifetimeManager::PushFunction(glfwTerminate);
		LifetimeManager::PushFunction(glfwDestroyWindow, m_Window);
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_Window);
	}

}