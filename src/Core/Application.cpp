#include "Application.h"
#include "VulkanCore.h"
#include "CommandsExecutor.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

namespace tiny_vulkan {

	Application::Application(const ApplicationSpec& appSpec)
	{
		LogSystem::Initialize();

		m_Window = std::make_shared<Window>(appSpec.windowWidth, appSpec.windowHeight, appSpec.windowName);

		VulkanCore::Initialize(m_Window);

		CommandExecutor::Initialize();

		m_Renderer = std::make_shared<VulkanRenderer>(m_Window);
	}

	Application::~Application() 
	{
		LifetimeManager::PushFunction(vkDeviceWaitIdle, VulkanCore::GetDevice());
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