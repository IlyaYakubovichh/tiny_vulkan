#include "VulkanDescriptorPool.h"
#include "LifetimeManager.h"

namespace tiny_vulkan {

	//======================================================================
	// POOL
	//======================================================================
	VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, VkDescriptorPool pool)
		: m_Device(device), m_Pool(pool)
	{

	}

	std::shared_ptr<VulkanDescriptorSet>  VulkanDescriptorPool::AllocateSet(VkDescriptorSetLayout setLayout)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_Pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &setLayout;

		VkDescriptorSet set{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkAllocateDescriptorSets(m_Device, &allocInfo, &set));

		return std::make_shared<VulkanDescriptorSet>(set);
	}

	void VulkanDescriptorPool::Reset()
	{
		CHECK_VK_RES(vkResetDescriptorPool(m_Device, m_Pool, 0));
	}

	//======================================================================
	// BUILDER
	//======================================================================
	DescriptorPoolBuilder& DescriptorPoolBuilder::AddSize(VkDescriptorType type, uint32_t count)
	{
		m_PoolSizes.push_back({ type, count });
		return *this;
	}

	DescriptorPoolBuilder& DescriptorPoolBuilder::SetMaxSets(uint32_t maxSets)
	{
		m_MaxSets = maxSets;
		return *this;
	}

	std::shared_ptr<VulkanDescriptorPool> DescriptorPoolBuilder::Build(VkDevice device)
	{
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = m_MaxSets;
		poolInfo.poolSizeCount = static_cast<uint32_t>(m_PoolSizes.size());
		poolInfo.pPoolSizes = m_PoolSizes.data();

		VkDescriptorPool pool{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));

		LifetimeManager::PushFunction(vkDestroyDescriptorPool, device, pool, nullptr);

		return std::make_shared<VulkanDescriptorPool>(device, pool);
	}

}