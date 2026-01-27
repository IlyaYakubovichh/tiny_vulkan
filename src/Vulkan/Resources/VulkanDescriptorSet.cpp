#include "VulkanDescriptorSet.h"
#include "VulkanCore.h"

namespace tiny_vulkan {

	void VulkanDescriptorSet::WriteImage(const DescriptorWriteImageInfo& info)
	{
		auto device = VulkanCore::GetDevice();

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = info.imageView;
		imageInfo.imageLayout = info.imageLayout;
		imageInfo.sampler = info.sampler;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_Set;
		write.dstBinding = info.dstBinding;
		write.descriptorCount = info.descriptorCount;
		write.descriptorType = info.descriptorType;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

}