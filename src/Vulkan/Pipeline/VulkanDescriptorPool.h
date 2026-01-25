#pragma once

#include "VulkanDescriptorSet.h" 
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace tiny_vulkan {

	class VulkanDescriptorPool
	{
	public:
		VulkanDescriptorPool(VkDevice device, VkDescriptorPool pool);
		~VulkanDescriptorPool() = default;

		[[nodiscard]] VulkanDescriptorSet AllocateSet(VkDescriptorSetLayout setLayout);
		void Reset();

		[[nodiscard]] VkDescriptorPool GetRaw() const { return m_Pool; }

	private:
		VkDevice		 m_Device{ VK_NULL_HANDLE };
		VkDescriptorPool m_Pool{ VK_NULL_HANDLE };
	};

	class DescriptorPoolBuilder
	{
	public:
		DescriptorPoolBuilder() = default;

		DescriptorPoolBuilder& AddSize(VkDescriptorType type, uint32_t count);
		DescriptorPoolBuilder& SetMaxSets(uint32_t maxSets);
		[[nodiscard]] std::shared_ptr<VulkanDescriptorPool> Build(VkDevice device);

	private:
		std::vector<VkDescriptorPoolSize> m_PoolSizes;
		uint32_t m_MaxSets{ 1000 };
	};

}