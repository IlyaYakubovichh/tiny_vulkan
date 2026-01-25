#include "CommandsExecutor.h"
#include "VulkanCore.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

namespace tiny_vulkan {

	// Definition of static members
	VkDevice        CommandExecutor::s_Device = VK_NULL_HANDLE;
	VkQueue			CommandExecutor::s_GraphicsQueue;
	VkCommandPool   CommandExecutor::s_CommandPool = VK_NULL_HANDLE;
	VkCommandBuffer CommandExecutor::s_CommandBuffer = VK_NULL_HANDLE;
	VkFence			CommandExecutor::s_Fence;
	bool            CommandExecutor::s_Initialized = false;

	void CommandExecutor::Initialize(std::shared_ptr<VulkanCore> core) 
	{
		if (s_Initialized) return;

		s_Initialized = true;

		s_Device = core->GetDevice();

		// Pool
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = core->GetGraphicsFamily();
		CHECK_VK_RES(vkCreateCommandPool(s_Device, &poolInfo, nullptr, &s_CommandPool));

		// Allocate
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = s_CommandPool;
		allocInfo.commandBufferCount = 1;
		CHECK_VK_RES(vkAllocateCommandBuffers(s_Device, &allocInfo, &s_CommandBuffer));

		// Fence 
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		CHECK_VK_RES(vkCreateFence(s_Device, &fenceInfo, nullptr, &s_Fence));

		// Cleanup
		LifetimeManager::PushFunction(vkDestroyFence, s_Device, s_Fence, nullptr);
		LifetimeManager::PushFunction(vkDestroyCommandPool, s_Device, s_CommandPool, nullptr);
	}

	void CommandExecutor::Execute(std::function<void(VkCommandBuffer cmd)>&& func)
	{
		if (!s_Initialized) 
		{
			LOG_ERROR("Immediate command executor is not initialized!");
			abort();
		}

		// Recording
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		CHECK_VK_RES(vkBeginCommandBuffer(s_CommandBuffer, &beginInfo));

		if (func) 
		{
			func(s_CommandBuffer);
		}

		CHECK_VK_RES(vkEndCommandBuffer(s_CommandBuffer));

		// Submit
		VkCommandBufferSubmitInfo cmdInfo = {};
		cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdInfo.commandBuffer = s_CommandBuffer;

		VkSubmitInfo2 submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &cmdInfo;
		CHECK_VK_RES(vkQueueSubmit2(s_GraphicsQueue, 1, &submitInfo, s_Fence));

		// Wait
		CHECK_VK_RES(vkWaitForFences(s_Device, 1, &s_Fence, VK_TRUE, UINT64_MAX));
	}
}