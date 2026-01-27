#include "VulkanFrame.h"
#include "VulkanCore.h"
#include "LifetimeManager.h"

namespace tiny_vulkan {

	std::vector<VkSemaphore> VulkanFrame::s_RenderSemaphores;

	VulkanFrame::VulkanFrame()
	{
		auto device					= VulkanCore::GetDevice();
		auto graphicsFamily			= VulkanCore::GetGraphicsFamily();
		auto swapchainImageCount	= VulkanCore::GetSwapchain()->GetImages().size();

		// ========================================================
		// VkCommandPool
		// ========================================================
		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = graphicsFamily;

		CHECK_VK_RES(vkCreateCommandPool(device, &poolInfo, nullptr, &m_Pool));

		// ========================================================
		// VkCommandBuffer
		// ========================================================
		VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocInfo.commandPool = m_Pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		CHECK_VK_RES(vkAllocateCommandBuffers(device, &allocInfo, &m_CmdBuffer));

		// ========================================================
		// Synchronization (Per Frame)
		// ========================================================
		VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		CHECK_VK_RES(vkCreateFence(device, &fenceInfo, nullptr, &m_RenderFence));

		VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		CHECK_VK_RES(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAcquireSemaphore));

		// ========================================================
		// Static Synchronization (Per Swapchain Image)
		// ========================================================
		if (s_RenderSemaphores.empty()) [[unlikely]]
		{
			s_RenderSemaphores.resize(swapchainImageCount);

			for (uint32_t i = 0; i < swapchainImageCount; ++i)
			{
				CHECK_VK_RES(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &s_RenderSemaphores[i]));
				LifetimeManager::PushFunction(vkDestroySemaphore, device, s_RenderSemaphores[i], nullptr);
			}
		}

		// ========================================================
		// Register cleanup for instance resources
		// ========================================================
		LifetimeManager::PushFunction(vkDestroyCommandPool, device, m_Pool, nullptr);
		LifetimeManager::PushFunction(vkDestroyFence, device, m_RenderFence, nullptr);
		LifetimeManager::PushFunction(vkDestroySemaphore, device, m_ImageAcquireSemaphore, nullptr);
	}

}