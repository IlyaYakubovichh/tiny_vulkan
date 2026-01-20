#pragma once

#include "VulkanDescriptors.h"
#include "VulkanPipelines.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <array>
#include <string>

namespace tiny_vulkan {

	struct EffectParams
	{
		EffectParams() { values.fill(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); }

		std::array<glm::vec4, 4> values;
	};

	struct ComputeEffect
	{
		std::string name;
		std::shared_ptr<VulkanPipeline> computePipeline;
		EffectParams params;

		static int m_SelectedEffectIndex;
	};

	class RendererData
	{
	public:
		void Initialize();
		void UpdateDescriptors(); 

		auto& GetEffects()			const { return m_ComputeEffects; }
		auto& GetCurrentEffect()	const { return m_ComputeEffects[ComputeEffect::m_SelectedEffectIndex]; }
		auto  GetDescriptorSet()	const { return m_DescriptorSet->GetRaw(); }

	private:
		std::vector<ComputeEffect> m_ComputeEffects;

		VkDescriptorSetLayout                 m_SetLayout{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanDescriptorPool> m_DescriptorPool;
		std::shared_ptr<VulkanDescriptorSet>  m_DescriptorSet;

		void CreatePipelines();
		void CreateDescriptors();
	};

}