#include "VulkanFrame.h"
#include "LifetimeManager.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"

namespace tiny_vulkan {

	std::vector<VkSemaphore> VulkanFrame::s_RenderSemaphores;
	bool VulkanFrame::s_RenderSemaphoresInitialized = false;

	VulkanFrame::VulkanFrame(VkDevice device, uint32_t queueFamilyIndex)
	{
		// ========================================================
		// VkCommandPool
		// ========================================================
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		CHECK_VK_RES(vkCreateCommandPool(device, &poolInfo, nullptr, &m_Pool));

		// ========================================================
		// VkCommandBuffer
		// ========================================================
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = m_Pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		vkAllocateCommandBuffers(device, &allocInfo, &m_CmdBuffer);

		// ========================================================
		// VkFence
		// ========================================================
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fenceInfo, nullptr, &m_RenderFence);

		// ========================================================
		// VkSemaphore
		// ========================================================
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAcquireSemaphore);

		// Per image semaphores
		if (!s_RenderSemaphoresInitialized)
		{
			const auto& images = VulkanRenderer::GetCore()->GetSwapchain()->GetImages();
			size_t imagesSize = images.size();
			s_RenderSemaphores.resize(imagesSize);

			for (int i = 0; i < imagesSize; ++i)
			{
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &s_RenderSemaphores[i]);
				LifetimeManager::PushFunction(vkDestroySemaphore, device, s_RenderSemaphores[i], nullptr);
			}
			s_RenderSemaphoresInitialized = true;
		}

		// ========================================================
		// Register cleanup
		// ========================================================
		LifetimeManager::PushFunction(vkDestroyCommandPool, device, m_Pool, nullptr);
		LifetimeManager::PushFunction(vkDestroySemaphore, device, m_ImageAcquireSemaphore, nullptr);
		LifetimeManager::PushFunction(vkDestroyFence, device, m_RenderFence, nullptr);
	}

}