#pragma once

#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

namespace tiny_vulkan { class VulkanCore; }

namespace tiny_vulkan {

	class CommandExecutor 
	{
	public:
		CommandExecutor() = delete;

		static void Initialize(std::shared_ptr<VulkanCore> core);
		static void Execute(std::function<void(VkCommandBuffer cmd)>&& func);

	private:
		static VkDevice			s_Device;
		static VkQueue			s_GraphicsQueue;
		static VkCommandPool	s_CommandPool;
		static VkCommandBuffer	s_CommandBuffer;
		static VkFence			s_Fence;
		static bool				s_Initialized;
	};

}