#include "VulkanDescriptors.h"
#include "VulkanRenderer.h"
#include "LifetimeManager.h"
#include "VulkanUtils.h"

namespace tiny_vulkan {

	// ========================================================
	// Layout
	// ========================================================
	// Builder
	VkDescriptorSetLayoutBuilder& VkDescriptorSetLayoutBuilder::AddBinding(uint32_t binding, uint32_t descriptorCount, VkDescriptorType descriptorType)
	{
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = descriptorCount;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

		m_Bindings.push_back(layoutBinding);

		return *this;
	}

	VkDescriptorSetLayout VkDescriptorSetLayoutBuilder::Build()
	{
		auto core = VulkanRenderer::GetCore();
		VkDevice device = core->GetDevice();

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.bindingCount = (uint32_t) m_Bindings.size();
		layoutInfo.pBindings = m_Bindings.data();

		VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));

		LifetimeManager::PushFunction(vkDestroyDescriptorSetLayout, device, layout, nullptr);

		return layout;
	}

	// ========================================================
	// Descriptor pool
	// ========================================================
	// VulkanDescriptorPool
	VulkanDescriptorPool::VulkanDescriptorPool(VkDescriptorPool pool)
		: m_Pool(pool)
	{

	}

	std::shared_ptr<VulkanDescriptorSet> VulkanDescriptorPool::AllocateSet(VkDescriptorSetLayout setLayout)
	{
		auto core = VulkanRenderer::GetCore();
		VkDevice device = core->GetDevice();

		VkDescriptorSetAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.descriptorPool = m_Pool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &setLayout;

		VkDescriptorSet set{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkAllocateDescriptorSets(device, &allocateInfo, &set));

		return std::make_shared<VulkanDescriptorSet>(set);
	}

	void VulkanDescriptorPool::ClearSets()
	{
		auto core = VulkanRenderer::GetCore();
		VkDevice device = core->GetDevice();

		CHECK_VK_RES(vkResetDescriptorPool(device, m_Pool, 0));
	}

	// Builder
	VulkanDescriptorPoolBuilder& VulkanDescriptorPoolBuilder::AddRatio(uint32_t descriptorCount, VkDescriptorType descriptorType)
	{
		m_PoolSizes.push_back(VkDescriptorPoolSize
			{
				.type = descriptorType, 
				.descriptorCount = descriptorCount 
			});
		return *this;
	}

	VulkanDescriptorPoolBuilder& VulkanDescriptorPoolBuilder::AddMaxSets(uint32_t maxSets)
	{
		m_MaxSets = maxSets;
		return *this;
	}

	std::shared_ptr<VulkanDescriptorPool> VulkanDescriptorPoolBuilder::Build()
	{
		auto core = VulkanRenderer::GetCore();
		VkDevice device = core->GetDevice();

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.flags = 0;
		poolInfo.maxSets = m_MaxSets;
		poolInfo.poolSizeCount = (uint32_t) m_PoolSizes.size();
		poolInfo.pPoolSizes = m_PoolSizes.data();

		VkDescriptorPool pool{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));

		LifetimeManager::PushFunction(vkDestroyDescriptorPool, device, pool, nullptr);

		return std::make_shared<VulkanDescriptorPool>(pool);
	}

	// ========================================================
	// Descriptor set
	// ========================================================
	// Set
	VulkanDescriptorSet::VulkanDescriptorSet(VkDescriptorSet set)
		: m_Set(set)
	{

	}

	void VulkanDescriptorSet::WriteImageDescriptor(const WriteImagePackage& package)
	{
		auto core = VulkanRenderer::GetCore();
		VkDevice device = core->GetDevice();

		VkDescriptorImageInfo descriptorImageInfo = {};
		descriptorImageInfo.imageView = package.imageView;
		descriptorImageInfo.imageLayout = package.imageLayout;
		
		VkWriteDescriptorSet imageWrite = {};
		imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageWrite.pNext = nullptr;
		imageWrite.dstSet = m_Set;
		imageWrite.dstBinding = package.dstBinding;
		imageWrite.descriptorCount = package.descriptorCount;
		imageWrite.descriptorType = package.descriptorType;
		imageWrite.pImageInfo = &descriptorImageInfo;

		vkUpdateDescriptorSets(device, 1, &imageWrite, 0, nullptr);
	}

}