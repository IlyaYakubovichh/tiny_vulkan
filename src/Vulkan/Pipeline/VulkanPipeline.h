#pragma once

#include "VulkanShader.h" 
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace tiny_vulkan {

	enum class PipelineType
	{
		GRAPHICS,
		COMPUTE,
		RAY_TRACING
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
	class PipelineBuilder
	{
	public:
		PipelineBuilder() = default;

		PipelineBuilder& SetPipelineType(PipelineType type);

		// Layout Setup
		PipelineBuilder& AddDescriptorLayout(VkDescriptorSetLayout layout);
		PipelineBuilder& AddPushConstantRange(VkPushConstantRange range);

		// Shader Stages
		PipelineBuilder& AddShader(std::shared_ptr<VulkanShader> shader);

		// Graphics Configuration
		PipelineBuilder& SetColorAttachmentFormats(const std::vector<VkFormat>& formats);
		PipelineBuilder& SetDepthFormat(VkFormat format);
		PipelineBuilder& SetTopology(VkPrimitiveTopology topology);
		PipelineBuilder& SetPolygonMode(VkPolygonMode mode);
		PipelineBuilder& SetCullMode(VkCullModeFlags cullMode);
		PipelineBuilder& SetFrontFace(VkFrontFace frontFace);

		// Build
		[[nodiscard]] std::shared_ptr<VulkanPipeline> Build(VkDevice device);

	private:
		bool BuildPipelineLayout(VkDevice device);
		std::shared_ptr<VulkanPipeline> BuildCompute(VkDevice device);
		std::shared_ptr<VulkanPipeline> BuildGraphics(VkDevice device);

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
	};

}