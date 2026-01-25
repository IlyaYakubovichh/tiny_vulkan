#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <GLFW/glfw3.h>

namespace tiny_vulkan {

	class Window
	{
	public:
		Window(uint32_t width = 1280, uint32_t height = 720, const std::string& title = "Tiny Vulkan");
		~Window() = default; 

		[[nodiscard]] bool			ShouldClose()	const;
		[[nodiscard]] GLFWwindow*	GetRaw()		const { return m_Window; }
		[[nodiscard]] uint32_t		GetWidth()		const { return m_Width; }
		[[nodiscard]] uint32_t		GetHeight()		const { return m_Height; }
		[[nodiscard]] void			SetWidth(uint32_t width)	{ m_Width = width; }
		[[nodiscard]] void			SetHeight(uint32_t height)	{ m_Height = height; }

		void OnUpdate();

	private:
		GLFWwindow* m_Window{ nullptr };
		uint32_t    m_Width{ 0 };
		uint32_t    m_Height{ 0 };
	};

}