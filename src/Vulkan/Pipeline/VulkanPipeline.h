#pragma once

#include "VulkanShader.h" 

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	enum class PipelineType
	{
		GRAPHICS,
		COMPUTE,
		RAY_TRACING
	};

	enum class BlendMode
	{
		NONE,       
		ALPHA,      
		ADDITIVE 
	};

	// ========================================================
	// Vulkan Pipeline 
	// ========================================================
	class VulkanPipeline
	{
	public:
		explicit VulkanPipeline(VkPipeline pipeline, VkPipelineLayout layout);
		~VulkanPipeline() = default;

		[[nodiscard]] VkPipeline       GetRaw()    const { return m_Pipeline; }
		[[nodiscard]] VkPipelineLayout GetLayout() const { return m_PipelineLayout; }

	private:
		VkPipeline       m_Pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
	};

	// ========================================================
	// Pipeline Builder
	// ========================================================
	class VulkanPipelineBuilder
	{
	public:
		VulkanPipelineBuilder() = default;

		[[nodiscard]] VulkanPipelineBuilder& SetPipelineType(PipelineType type);

		// Layout Setup
		[[nodiscard]] VulkanPipelineBuilder& AddDescriptorLayout(VkDescriptorSetLayout layout);
		[[nodiscard]] VulkanPipelineBuilder& AddPushConstantRange(VkPushConstantRange range);

		// Shader Stages
		[[nodiscard]] VulkanPipelineBuilder& AddShader(std::shared_ptr<VulkanShader> shader);

		// Graphics Configuration
		[[nodiscard]] VulkanPipelineBuilder& SetColorAttachmentFormats(const std::vector<VkFormat>& formats);
		[[nodiscard]] VulkanPipelineBuilder& SetDepthFormat(VkFormat format);
		[[nodiscard]] VulkanPipelineBuilder& SetTopology(VkPrimitiveTopology topology);
		[[nodiscard]] VulkanPipelineBuilder& SetPolygonMode(VkPolygonMode mode);
		[[nodiscard]] VulkanPipelineBuilder& SetCullMode(VkCullModeFlags cullMode);
		[[nodiscard]] VulkanPipelineBuilder& SetFrontFace(VkFrontFace frontFace);

		// Depth & Blend Configuration
		[[nodiscard]] VulkanPipelineBuilder& EnableDepthTest(bool enable);
		[[nodiscard]] VulkanPipelineBuilder& SetBlendMode(BlendMode mode);

		// Build
		[[nodiscard]] std::shared_ptr<VulkanPipeline> Build();

	private:
		[[nodiscard]] bool BuildPipelineLayout();
		[[nodiscard]] std::shared_ptr<VulkanPipeline> BuildCompute();
		[[nodiscard]] std::shared_ptr<VulkanPipeline> BuildGraphics();

	private:
		PipelineType									m_Type{ PipelineType::GRAPHICS };
		VkPipelineLayout								m_PipelineLayout{ VK_NULL_HANDLE };

		std::vector<std::shared_ptr<VulkanShader>>		m_Shaders;
		std::vector<VkDescriptorSetLayout>				m_DescriptorSetLayouts;
		std::vector<VkPushConstantRange>				m_Ranges;

		// Graphics State
		std::vector<VkFormat>							m_ColorFormats;
		VkFormat										m_DepthFormat{ VK_FORMAT_UNDEFINED };
		VkPrimitiveTopology								m_Topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
		VkPolygonMode									m_PolygonMode{ VK_POLYGON_MODE_FILL };
		VkCullModeFlags									m_CullMode{ VK_CULL_MODE_BACK_BIT };
		VkFrontFace										m_FrontFace{ VK_FRONT_FACE_CLOCKWISE };

		// Depth & Blend State
		bool											m_DepthTestEnable{ true };
		bool											m_DepthWriteEnable{ true };
		BlendMode										m_BlendMode{ BlendMode::NONE };
	};

}