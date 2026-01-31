#include "Window.h"
#include "Application.h"
#include "VulkanCore.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

namespace tiny_vulkan {

	static void GLFWErrorCallback(int error, const char* description)
	{
		LOG_ERROR(fmt::runtime("GLFW Error ({0}): {1}"), error, description);
	}

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) // will be called when the size in pixels change
	{
		VulkanCore::GetSwapchain()->RecreateSwapchain(width, height);
		LOG_INFO(fmt::runtime("Window resized to: {0}x{1}"), width, height);
	}

	Window::Window(uint32_t width /*= 1280*/, uint32_t height /*= 720*/, const std::string& title /*= "Tiny Vulkan"*/)
		: m_Width(width), m_Height(height)
	{
		LOG_INFO(fmt::runtime("Creating Window: {0} ({1}x{2})"), title, width, height);

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

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
		else
		{
			glfwSetErrorCallback(GLFWErrorCallback);
			glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
		}

		LifetimeManager::PushFunction(glfwTerminate);
		LifetimeManager::PushFunction(glfwDestroyWindow, m_Window);
	}

	void Window::Update()
	{
		glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
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