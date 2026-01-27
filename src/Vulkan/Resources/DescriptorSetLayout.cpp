#include "DescriptorSetLayout.h"
#include "LifetimeManager.h" 

namespace tiny_vulkan {

	DescriptorLayoutBuilder& DescriptorLayoutBuilder::AddBinding(uint32_t binding, uint32_t descriptorCount, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = descriptorCount;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = stageFlags;

		m_Bindings.push_back(layoutBinding);
		return *this;
	}

	VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
		layoutInfo.pBindings = m_Bindings.data();

		VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));

		LifetimeManager::PushFunction(vkDestroyDescriptorSetLayout, device, layout, nullptr);

		return layout;
	}

}