#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	// ========================================================
	// Layout
	// ========================================================
	// Layout
	class VkDescriptorSetLayoutBuilder
	{
	public:
		VkDescriptorSetLayoutBuilder() = default;
		~VkDescriptorSetLayoutBuilder() = default;

		VkDescriptorSetLayoutBuilder& AddBinding(uint32_t binding, uint32_t descriptorCount, VkDescriptorType descriptorType);
		VkDescriptorSetLayout Build();

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
	};

	// ========================================================
	// Descriptor pool
	// ========================================================
	// Pool
	class VulkanDescriptorSet;

	class VulkanDescriptorPool
	{
	public:
		VulkanDescriptorPool(VkDescriptorPool pool);
		~VulkanDescriptorPool() = default;

		std::shared_ptr<VulkanDescriptorSet> AllocateSet(VkDescriptorSetLayout setLayout);
		void ClearSets();

		auto GetRaw() const { return m_Pool; }

	private:
		VkDescriptorPool m_Pool{ VK_NULL_HANDLE };
	};

	// Builder
	class VulkanDescriptorPoolBuilder
	{
	public:
		VulkanDescriptorPoolBuilder() = default;
		~VulkanDescriptorPoolBuilder() = default;

		VulkanDescriptorPoolBuilder& AddRatio(uint32_t descriptorCount, VkDescriptorType descriptorType);
		VulkanDescriptorPoolBuilder& AddMaxSets(uint32_t maxSets);
		std::shared_ptr<VulkanDescriptorPool> Build();

	private:
		std::vector<VkDescriptorPoolSize> m_PoolSizes;
		uint32_t m_MaxSets{ 0 };
	};

	// ========================================================
	// Descriptor set
	// ========================================================
	// Set

	struct WriteImagePackage
	{
		VkImageView			imageView{ VK_NULL_HANDLE };					// descriptor will point to this view
		VkImageLayout		imageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };		// layout of image when we access it
		uint32_t			dstBinding{ 0 };								// at what binding 'write' will happen
		uint32_t			descriptorCount{ 0 };							// how much descriptors we write (pointers to some resources)
		VkDescriptorType	descriptorType{VK_DESCRIPTOR_TYPE_MAX_ENUM};	// what type of descriptor we write
	};

	class VulkanDescriptorSet
	{
	public:
		VulkanDescriptorSet(VkDescriptorSet set);
		~VulkanDescriptorSet() = default;

		void WriteImageDescriptor(const WriteImagePackage& package);

		auto GetRaw() const { return m_Set; }

	private:
		VkDescriptorSet m_Set{ VK_NULL_HANDLE };
	};
}