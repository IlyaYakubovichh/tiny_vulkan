#include "VulkanPipelines.h"
#include "VulkanRenderer.h"
#include "LifetimeManager.h"
#include "VulkanUtils.h"

namespace tiny_vulkan {

	VkPipelineBuilder& VkPipelineBuilder::SetPipelineType(PipelineType type)
	{
		m_Type = type;
		return *this;
	}

	VkPipelineBuilder& VkPipelineBuilder::LayoutAddDescriptorLayout(VkDescriptorSetLayout layout)
	{
		m_DescriptorSetLayout = layout;
		return *this;
	}

	VkPipelineBuilder& VkPipelineBuilder::AddShader(std::shared_ptr<VulkanShader> shader)
	{
		m_Shader = shader;
		return *this;
	}

	std::shared_ptr<VulkanPipeline> VkPipelineBuilder::Build()
	{
		switch (m_Type)
		{
		case PipelineType::COMPUTE: return BuildCompute();
		}
	}

	std::shared_ptr<VulkanPipeline> VkPipelineBuilder::BuildCompute()
	{
		auto core = VulkanRenderer::GetCore();
		VkDevice device = core->GetDevice();

		// ========================================================
		// VkPipelineLayout
		// ========================================================
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		pipelineLayoutInfo.setLayoutCount = 1;

		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

		// ========================================================
		// Shaders
		// ========================================================
		VkPipelineShaderStageCreateInfo stageInfo = {};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.pNext = nullptr;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = m_Shader->GetRaw();
		stageInfo.pName = "main";

		// ========================================================
		// VkPipeline
		// ========================================================
		VkComputePipelineCreateInfo computePipelineInfo = {};
		computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineInfo.pNext = nullptr;
		computePipelineInfo.layout = pipelineLayout;
		computePipelineInfo.stage = stageInfo;
		
		VkPipeline pipeline{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &pipeline));

		// ========================================================
		// Cleanup
		// ========================================================
		LifetimeManager::PushFunction(vkDestroyPipeline, device, pipeline, nullptr);
		LifetimeManager::PushFunction(vkDestroyPipelineLayout, device, pipelineLayout, nullptr);

		return std::make_shared<VulkanPipeline>(pipeline, pipelineLayout);
	}

	VulkanPipeline::VulkanPipeline(VkPipeline pipeline, VkPipelineLayout pipelineLayout)
		: m_Pipeline(pipeline),
		m_PipelineLayout(pipelineLayout)
	{

	}

	VulkanPipeline::~VulkanPipeline()
	{

	}

}