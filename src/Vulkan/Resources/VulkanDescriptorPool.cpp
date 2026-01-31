#include "VulkanDescriptorPool.h"
#include "VulkanCore.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

namespace tiny_vulkan {

	constexpr int perPoolMaxSets = 4092;

	void DescriptorAllocator::Initialize(uint32_t setsPerPool, const std::vector<PoolSizeRatio>& ratios)
	{
		if (setsPerPool > perPoolMaxSets)
		{
			LOG_WARN("You want to create setsPerPoolCount that > 4092, using 4092!");
			m_SetsPerPool = perPoolMaxSets;
		}

		m_SetsPerPool = setsPerPool;
		m_Ratios = ratios;
	}

	void DescriptorAllocator::ResetPools()
	{
		auto device = VulkanCore::GetDevice();

		// Clear sets from readyPools
		for (auto readyPool : m_ReadyPools)
		{
			vkResetDescriptorPool(device, readyPool, 0);
		}

		// Clear sets from full pools and then add them to ready
		for (auto fullPool : m_FullPools)
		{
			vkResetDescriptorPool(device, fullPool, 0);
			m_ReadyPools.push_back(fullPool);
		}
		m_FullPools.clear();
	}

	void DescriptorAllocator::DestroyPools()
	{
		auto device = VulkanCore::GetDevice();

		// Clear ready pools
		for (auto readyPool : m_ReadyPools)
		{
			LifetimeManager::ExecuteNow(vkDestroyDescriptorPool, device, readyPool, nullptr);
		}
		m_ReadyPools.clear();

		// Clear full pools
		for (auto fullPool : m_FullPools)
		{
			LifetimeManager::ExecuteNow(vkDestroyDescriptorPool, device, fullPool, nullptr);
		}
		m_FullPools.clear();
	}

	std::shared_ptr<VulkanDescriptorSet> DescriptorAllocator::Allocate(VkDescriptorSetLayout layout)
	{
		auto device = VulkanCore::GetDevice();

		VkDescriptorPool poolToUse = GetPool();

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = poolToUse;
		allocInfo.pSetLayouts = &layout;
		allocInfo.descriptorSetCount = 1;

		VkDescriptorSet set{ VK_NULL_HANDLE };
		VkResult res = vkAllocateDescriptorSets(device, &allocInfo, &set);
		if (res == VK_ERROR_OUT_OF_POOL_MEMORY || res == VK_ERROR_FRAGMENTED_POOL)
		{
			m_FullPools.push_back(poolToUse);

			poolToUse = GetPool();
			allocInfo.descriptorPool = poolToUse;

			CHECK_VK_RES(vkAllocateDescriptorSets(device, &allocInfo, &set));
		}

		m_ReadyPools.push_back(poolToUse);

		return std::make_shared<VulkanDescriptorSet>(set);
	}

	VkDescriptorPool DescriptorAllocator::GetPool()
	{
		VkDescriptorPool pool{ VK_NULL_HANDLE };

		// If have pool that we can allocate from
		if (!m_ReadyPools.empty())
		{
			pool = m_ReadyPools.back();
			m_ReadyPools.pop_back();
		}
		// If don't have, then create new pool
		else
		{
			pool = CreatePool(m_SetsPerPool, m_Ratios);
		}

		return pool;
	}

	VkDescriptorPool DescriptorAllocator::CreatePool(uint32_t maxSets, const std::vector<PoolSizeRatio>& ratios)
	{
		auto device = VulkanCore::GetDevice();

		std::vector<VkDescriptorPoolSize> poolSizes(m_Ratios.size());
		for (const auto& ratio : ratios)
		{
			poolSizes.push_back(VkDescriptorPoolSize{
				.type = ratio.type,
				.descriptorCount = (uint32_t)ratio.count * m_SetsPerPool
				});
		}

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.flags = 0;
		poolInfo.maxSets = maxSets;
		poolInfo.poolSizeCount = (uint32_t) m_Ratios.size();
		poolInfo.pPoolSizes = poolSizes.data();

		VkDescriptorPool pool{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));

		return pool;
	}

}