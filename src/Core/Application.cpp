#include "Application.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

namespace tiny_vulkan {

	Application::Application() 
	{
		LogSystem::Initialize();
		m_Window = std::make_shared<Window>(1280, 720, "TinyVulkan");
		m_Core = std::make_shared<VulkanCore>(m_Window); 
		m_Renderer = std::make_shared<VulkanRenderer>(m_Window, m_Core);
	}

	Application::~Application() 
	{
		LifetimeManager::PushFunction(vkDeviceWaitIdle, m_Core->GetDevice());
		LifetimeManager::ExecuteAll(); 
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose()) 
		{
			m_Window->OnUpdate();
		}
	}
}