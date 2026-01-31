#pragma once

#include "VulkanDescriptorSet.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace tiny_vulkan {

	struct PoolSizeRatio
	{
		VkDescriptorType type;
		uint32_t count;
	};

	class DescriptorAllocator
	{
	public:
		DescriptorAllocator() = default;
		~DescriptorAllocator() = default;
		DescriptorAllocator(const DescriptorAllocator&) = delete;
		DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;

		void Initialize(uint32_t setsPerPool, const std::vector<PoolSizeRatio>& ratios);
		void ResetPools();
		void DestroyPools();

		std::shared_ptr<VulkanDescriptorSet> Allocate(VkDescriptorSetLayout layout);

	private:
		VkDescriptorPool GetPool();
		VkDescriptorPool CreatePool(uint32_t maxSets, const std::vector<PoolSizeRatio>& ratios);

	private:
		uint32_t						m_SetsPerPool{ 1000 };
		std::vector<PoolSizeRatio>		m_Ratios;
		std::vector<VkDescriptorPool>	m_FullPools;
		std::vector<VkDescriptorPool>	m_ReadyPools;
	};

}