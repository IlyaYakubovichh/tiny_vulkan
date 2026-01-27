#include "VulkanPipeline.h"
#include "VulkanCore.h"
#include "LifetimeManager.h" 

namespace tiny_vulkan {

	// ==============================================================================
	// VulkanPipeline Implementation
	// ==============================================================================
	VulkanPipeline::VulkanPipeline(VkPipeline pipeline, VkPipelineLayout layout)
		: m_Pipeline(pipeline)
		, m_PipelineLayout(layout)
	{

	}

	// ==============================================================================
	// PipelineBuilder 
	// ==============================================================================
	VulkanPipelineBuilder& VulkanPipelineBuilder::SetPipelineType(PipelineType type)
	{
		m_Type = type;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::AddDescriptorLayout(VkDescriptorSetLayout layout)
	{
		m_DescriptorSetLayouts.push_back(layout);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::AddPushConstantRange(VkPushConstantRange range)
	{
		m_Ranges.push_back(range);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::AddShader(std::shared_ptr<VulkanShader> shader)
	{
		if (shader) 
		{
			m_Shaders.push_back(shader);
		}
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::SetColorAttachmentFormats(const std::vector<VkFormat>& formats)
	{
		m_ColorFormats = formats;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::SetDepthFormat(VkFormat format)
	{
		m_DepthFormat = format;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::SetTopology(VkPrimitiveTopology topology)
	{
		m_Topology = topology;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::SetPolygonMode(VkPolygonMode mode)
	{
		m_PolygonMode = mode;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::SetCullMode(VkCullModeFlags cullMode)
	{
		m_CullMode = cullMode;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::SetFrontFace(VkFrontFace frontFace)
	{
		m_FrontFace = frontFace;
		return *this;
	}

	std::shared_ptr<VulkanPipeline> VulkanPipelineBuilder::Build()
	{
		auto device = VulkanCore::GetDevice();

		// Create pipeline layout
		if (!BuildPipelineLayout()) 
		{
			return nullptr;
		}

		// Create specific pipeline
		switch (m_Type)
		{
		case PipelineType::COMPUTE:  return BuildCompute();
		case PipelineType::GRAPHICS: return BuildGraphics();
		default:
			return nullptr;
		}
	}

	bool VulkanPipelineBuilder::BuildPipelineLayout()
	{
		auto device = VulkanCore::GetDevice();

		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pSetLayouts = m_DescriptorSetLayouts.data();
		info.setLayoutCount = (uint32_t)m_DescriptorSetLayouts.size();
		info.pPushConstantRanges = m_Ranges.data();
		info.pushConstantRangeCount = (uint32_t)m_Ranges.size();

		CHECK_VK_RES(vkCreatePipelineLayout(device, &info, nullptr, &m_PipelineLayout));

		LifetimeManager::PushFunction(vkDestroyPipelineLayout, device, m_PipelineLayout, nullptr);

		return true;
	}

	std::shared_ptr<VulkanPipeline> VulkanPipelineBuilder::BuildCompute()
	{
		auto device = VulkanCore::GetDevice();

		if (m_Shaders.empty()) return nullptr;

		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = m_Shaders[0]->GetRaw();
		stageInfo.pName = "main";

		VkComputePipelineCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.layout = m_PipelineLayout;
		info.stage = stageInfo;

		VkPipeline pipeline{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline));

		LifetimeManager::PushFunction(vkDestroyPipeline, device, pipeline, nullptr);

		return std::make_shared<VulkanPipeline>(pipeline, m_PipelineLayout);
	}

	std::shared_ptr<VulkanPipeline> VulkanPipelineBuilder::BuildGraphics()
	{
		auto device = VulkanCore::GetDevice();

		// Shaders
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(m_Shaders.size());
		for (const auto& shader : m_Shaders) 
		{
			VkPipelineShaderStageCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.module = shader->GetRaw();
			info.pName = "main";
			info.stage = shader->GetStage();
			shaderStages.push_back(info);
		}

		// Dynamic rendering
		VkPipelineRenderingCreateInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount = (uint32_t)m_ColorFormats.size();
		renderingInfo.pColorAttachmentFormats = m_ColorFormats.data();
		renderingInfo.depthAttachmentFormat = m_DepthFormat;

		// Input assembly
		VkPipelineInputAssemblyStateCreateInfo assemblyInfo{};
		assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyInfo.topology = m_Topology;
		assemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Rasterization 
		VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
		rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerInfo.polygonMode = m_PolygonMode;
		rasterizerInfo.lineWidth = 1.0f;
		rasterizerInfo.cullMode = m_CullMode;
		rasterizerInfo.frontFace = m_FrontFace;

		// Depth & Stencil
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		bool hasDepth = m_DepthFormat != VK_FORMAT_UNDEFINED;
		depthStencilInfo.depthTestEnable = hasDepth ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthWriteEnable = hasDepth ? VK_TRUE : VK_FALSE;
		depthStencilInfo.depthCompareOp = hasDepth ? VK_COMPARE_OP_LESS_OR_EQUAL : VK_COMPARE_OP_ALWAYS;
		depthStencilInfo.minDepthBounds = 0.0f;
		depthStencilInfo.maxDepthBounds = 1.0f;

		// Color Blending
		VkPipelineColorBlendAttachmentState defaultBlendAttachment{};
		defaultBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		defaultBlendAttachment.blendEnable = VK_FALSE;

		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(m_ColorFormats.size(), defaultBlendAttachment);

		VkPipelineColorBlendStateCreateInfo blendInfo{};
		blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendInfo.attachmentCount = (uint32_t)blendAttachments.size();
		blendInfo.pAttachments = blendAttachments.data();

		// Dynamic viewport
		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.scissorCount = 1;

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicInfo{};
		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.pDynamicStates = dynamicStates;
		dynamicInfo.dynamicStateCount = 2;

		// Multisample 
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// Vertex Input 
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		// Build 
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &renderingInfo;
		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &assemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &rasterizerInfo;
		pipelineInfo.pMultisampleState = &multisampleInfo;
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
		pipelineInfo.pColorBlendState = &blendInfo;
		pipelineInfo.pDynamicState = &dynamicInfo;
		pipelineInfo.layout = m_PipelineLayout;

		VkPipeline pipeline{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

		LifetimeManager::PushFunction(vkDestroyPipeline, device, pipeline, nullptr);

		return std::make_shared<VulkanPipeline>(pipeline, m_PipelineLayout);
	}

}