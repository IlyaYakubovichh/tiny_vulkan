#include "Window.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

namespace tiny_vulkan {

	static void GLFWErrorCallback(int error, const char* description)
	{
		LOG_ERROR(fmt::runtime("GLFW Error ({0}): {1}"), error, description);
	}

	Window::Window(uint32_t width /*= 1280*/, uint32_t height /*= 720*/, const std::string& title /*= "Tiny Vulkan"*/)
		: m_Width(width), m_Height(height)
	{
		LOG_INFO(fmt::runtime("Creating Window: {0} ({1}x{2})"), title, width, height);

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwSetErrorCallback(GLFWErrorCallback);

		m_Window = glfwCreateWindow(
			static_cast<int>(width),
			static_cast<int>(height),
			title.c_str(),
			nullptr,
			nullptr
		);

		if (!m_Window)
		{
			LOG_CRITICAL("Failed to create GLFW window");
		}

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