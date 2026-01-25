#pragma once

#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	struct DescriptorWriteImageInfo
	{
		VkImageView			imageView{ VK_NULL_HANDLE };
		VkImageLayout		imageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		uint32_t			dstBinding{ 0 };
		uint32_t			descriptorCount{ 1 };
		VkDescriptorType	descriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		VkSampler			sampler{ VK_NULL_HANDLE }; 
	};

	class VulkanDescriptorSet
	{
	public:
		VulkanDescriptorSet() = default;
		explicit VulkanDescriptorSet(VkDescriptorSet set) : m_Set(set) {}

		void WriteImage(VkDevice device, const DescriptorWriteImageInfo& info);

		[[nodiscard]] VkDescriptorSet GetRaw() const { return m_Set; }

	private:
		VkDescriptorSet m_Set{ VK_NULL_HANDLE };
	};

}