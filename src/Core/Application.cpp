#include "Application.h"
#include "VulkanCore.h"
#include "CommandsExecutor.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

namespace tiny_vulkan {

	Application* Application::s_AppInstance = nullptr;

	Application::Application(const ApplicationSpec& appSpec)
	{
		s_AppInstance = this;

		LogSystem::Initialize();

		m_Window = std::make_shared<Window>(appSpec.windowWidth, appSpec.windowHeight, appSpec.windowName);

		VulkanCore::Initialize(m_Window);

		CommandExecutor::Initialize();

		m_Renderer = std::make_shared<VulkanRenderer>(m_Window);
	}

	Application::~Application() 
	{
		LifetimeManager::ExecuteNow(vkDeviceWaitIdle, VulkanCore::GetDevice());
		VulkanCore::GetSwapchain()->CleanupResources();
		LifetimeManager::ExecuteAll(); 
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose()) 
		{
			m_Window->OnUpdate();
			m_Renderer->Draw();
		}
	}

}