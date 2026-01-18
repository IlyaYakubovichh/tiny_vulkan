#pragma once

#include <string>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace tiny_vulkan {

	class Window
	{
	public:
		Window();
		~Window() = default;

		[[nodiscard]] bool		   ShouldClose() const;
		[[nodiscard]] GLFWwindow*  GetRaw()		 const { return m_Window; }
		[[nodiscard]] uint32_t     GetWidth()	 const { return 1280; }
		[[nodiscard]] uint32_t     GetHeight()	 const { return 720; }

		void OnUpdate();

	private:
		GLFWwindow*	m_Window{ nullptr };
	};

}