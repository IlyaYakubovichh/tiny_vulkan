#pragma once

#include <string>
#include <cstdint>
#include <GLFW/glfw3.h>

namespace tiny_vulkan {

	class Window
	{
	public:
		Window(uint32_t width = 1280, uint32_t height = 720, const std::string& title = "Tiny Vulkan");
		~Window() = default; 

		[[nodiscard]] bool			ShouldClose()	const;
		[[nodiscard]] GLFWwindow*	GetRaw()		const	{ return m_Window; }
		[[nodiscard]] int			GetWidth()		const	{ return m_Width; }
		[[nodiscard]] int			GetHeight()		const	{ return m_Height; }
		void						SetWidth(int width)		{ m_Width = width; }
		void						SetHeight(int height)	{ m_Height = height; }
		void						Update();

		void						OnUpdate();

	private:
		GLFWwindow* m_Window{ nullptr };
		int m_Width{ 0 };
		int m_Height{ 0 };
	};

}