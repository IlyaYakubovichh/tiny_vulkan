#include "VulkanPipelines.h"
#include "VulkanRenderer.h"
#include "LifetimeManager.h"
#include "VulkanUtils.h"
#include "LogSystem.h" 

namespace tiny_vulkan {

	// ==============================================================================
	// VulkanPipelineBuilder Implementation
	// ==============================================================================
	VulkanPipelineBuilder& VulkanPipelineBuilder::SetPipelineType(PipelineType type)
	{
		m_Type = type;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::LayoutAddDescriptorLayout(VkDescriptorSetLayout layout)
	{
		m_DescriptorSetLayouts.push_back(layout);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::LayoutAddPushRange(VkPushConstantRange range)
	{
		m_Ranges.push_back(range);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::AddShader(std::shared_ptr<VulkanShader> shader)
	{
		if (shader) {
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
		auto core = VulkanRenderer::GetCore();
		VkDevice device = core->GetDevice();

		if (!BuildPipelineLayout(device))
		{
			LOG_CRITICAL("Failed to create Pipeline Layout");
			return nullptr;
		}

		switch (m_Type)
		{
		case PipelineType::COMPUTE: return BuildCompute(device);
		case PipelineType::GRAPHICS: return BuildGraphics(device);
		default: return nullptr;
		}
	}

	bool VulkanPipelineBuilder::BuildPipelineLayout(VkDevice device)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();
		pipelineLayoutInfo.setLayoutCount = (uint32_t)m_DescriptorSetLayouts.size();
		pipelineLayoutInfo.pPushConstantRanges = m_Ranges.data();
		pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)m_Ranges.size();

		CHECK_VK_RES(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

		LifetimeManager::PushFunction(vkDestroyPipelineLayout, device, m_PipelineLayout, nullptr);

		return true;
	}

	std::shared_ptr<VulkanPipeline> VulkanPipelineBuilder::BuildCompute(VkDevice device)
	{
		if (m_Shaders.empty()) {
			LOG_CRITICAL("No shaders provided for Compute Pipeline");
			return nullptr;
		}

		VkPipelineShaderStageCreateInfo stageInfo = {};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = m_Shaders[0]->GetRaw();
		stageInfo.pName = "main";

		VkComputePipelineCreateInfo computePipelineInfo = {};
		computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineInfo.layout = m_PipelineLayout;
		computePipelineInfo.stage = stageInfo;

		VkPipeline pipeline{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &pipeline));

		LifetimeManager::PushFunction(vkDestroyPipeline, device, pipeline, nullptr);

		return std::make_shared<VulkanPipeline>(pipeline, m_PipelineLayout);
	}

	std::shared_ptr<VulkanPipeline> VulkanPipelineBuilder::BuildGraphics(VkDevice device)
	{
		// ========================================================
		// Shader setup
		// ========================================================
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(m_Shaders.size());

		for (const auto& shader : m_Shaders)
		{
			VkPipelineShaderStageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.module = shader->GetRaw();
			info.pName = "main";
			info.stage = shader->GetStage();

			shaderStages.push_back(info);
		}

		// ========================================================
		// Dynamic rendering 
		// ========================================================
		std::vector<VkFormat> defaultColorFormats;
		if (m_ColorFormats.empty())
		{
			defaultColorFormats.push_back(VulkanRenderer::GetCore()->GetRenderTarget()->GetFormat());
		}

		const auto& useColorFormats = m_ColorFormats.empty() ? defaultColorFormats : m_ColorFormats;

		VkPipelineRenderingCreateInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount = (uint32_t)useColorFormats.size();
		renderingInfo.pColorAttachmentFormats = useColorFormats.data(); 
		renderingInfo.depthAttachmentFormat = m_DepthFormat;
		renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

		// ========================================================
		// Vertex input
		// ========================================================
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		// ========================================================
		// Assembly
		// ========================================================
		// what kind of geometry will be drawn from the vertices and if primitive restart should be enabled.
		VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {};
		assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyInfo.topology = m_Topology; 
		assemblyInfo.primitiveRestartEnable = VK_FALSE;

		// ========================================================
		// Viewport & scissors
		// ========================================================
		VkPipelineViewportStateCreateInfo viewportInfo = {};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.scissorCount = 1;

		// ========================================================
		// Rasterization
		// ========================================================
		VkPipelineRasterizationStateCreateInfo rasterizerInfo = {};
		rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerInfo.polygonMode = m_PolygonMode; 
		rasterizerInfo.lineWidth = 1.0f;
		rasterizerInfo.cullMode = m_CullMode; 
		rasterizerInfo.frontFace = m_FrontFace; 

		// ========================================================
		// Multisampling
		// ========================================================
		VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.sampleShadingEnable = VK_FALSE;
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleInfo.minSampleShading = 1.0f;
		multisampleInfo.pSampleMask = nullptr;
		multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleInfo.alphaToOneEnable = VK_FALSE;

		// ========================================================
		// Depth & stencil
		// ========================================================
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilInfo.depthTestEnable = VK_FALSE;
		depthStencilInfo.depthWriteEnable = VK_FALSE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_NEVER;
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
		depthStencilInfo.front = {};
		depthStencilInfo.back = {};
		depthStencilInfo.minDepthBounds = 0.0f;
		depthStencilInfo.maxDepthBounds = 1.0f;

		// ========================================================
		// Color blend
		// ========================================================
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo blendInfo = {};
		blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendInfo.logicOpEnable = VK_FALSE;
		blendInfo.attachmentCount = 1;
		blendInfo.pAttachments = &colorBlendAttachment;

		// ========================================================
		// Dynamic state
		// ========================================================
		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicInfo = {};
		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.pDynamicStates = dynamicStates;
		dynamicInfo.dynamicStateCount = 2;

		// ========================================================
		// Pipeline
		// ========================================================
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
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
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;

		VkPipeline pipeline{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

		LifetimeManager::PushFunction(vkDestroyPipeline, device, pipeline, nullptr);

		return std::make_shared<VulkanPipeline>(pipeline, m_PipelineLayout);
	}

	// ==============================================================================
	// VulkanPipeline Implementation
	// ==============================================================================

	VulkanPipeline::VulkanPipeline(VkPipeline pipeline, VkPipelineLayout pipelineLayout)
		: m_Pipeline(pipeline)
		, m_PipelineLayout(pipelineLayout)
	{

	}

	VulkanPipeline::~VulkanPipeline()
	{

	}
}