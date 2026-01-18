#include "VulkanRenderer.h"
#include "LogSystem.h"
#include "LifetimeManager.h"

namespace tiny_vulkan {

	std::shared_ptr<Window>		VulkanRenderer::m_Window;
	std::shared_ptr<VulkanCore> VulkanRenderer::m_VulkanCore;

	void VulkanRenderer::Initialize()
	{
		LogSystem::Initialize();

		m_Window		= std::make_shared<Window>();
		m_VulkanCore	= std::make_shared<VulkanCore>();
	}

	void VulkanRenderer::Shutdown()
	{
		LifetimeManager::ExecuteAll();
	}

	void VulkanRenderer::Run()
	{
		while (!m_Window->ShouldClose())
		{
			m_Window->OnUpdate();
		}
	}

}