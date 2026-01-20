#pragma once

#include "VulkanShader.h"
#include <memory>
#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	enum class PipelineType
	{
		GRAPHICS, 
		COMPUTE,
		RAY_TRACING
	};

	class VulkanPipeline;

	class VkPipelineBuilder
	{
	public:
		VkPipelineBuilder() = default;
		~VkPipelineBuilder() = default;

		VkPipelineBuilder& SetPipelineType(PipelineType type);
		VkPipelineBuilder& LayoutAddDescriptorLayout(VkDescriptorSetLayout layout);
		VkPipelineBuilder& AddShader(std::shared_ptr<VulkanShader> shader);
		std::shared_ptr<VulkanPipeline> Build();

	private:
		std::shared_ptr<VulkanPipeline> BuildCompute();

	private:
		PipelineType						m_Type;
		std::shared_ptr<VulkanShader>		m_Shader;
		std::vector<VkDescriptorSetLayout>	m_DescriptorSetLayouts;
	};

	class VulkanPipeline
	{
	public:
		VulkanPipeline(VkPipeline pipeline, VkPipelineLayout pipelineLayout);
		~VulkanPipeline();
		
		auto GetRaw() const { return m_Pipeline; }
		auto GetLayout() const { return m_PipelineLayout; }

	private:
		VkPipeline m_Pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
	};

}