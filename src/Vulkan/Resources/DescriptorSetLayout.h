#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	class DescriptorLayoutBuilder
	{
	public:
		explicit DescriptorLayoutBuilder() = default;

		[[nodiscard]] DescriptorLayoutBuilder& AddBinding(uint32_t binding, uint32_t descriptorCount, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);
		[[nodiscard]] VkDescriptorSetLayout Build(VkDevice device);

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
	};

}