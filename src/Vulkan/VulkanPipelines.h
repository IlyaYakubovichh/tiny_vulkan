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

	class VulkanPipeline;

	class VulkanPipelineBuilder
	{
	public:
		VulkanPipelineBuilder() = default;
		~VulkanPipelineBuilder() = default;

		VulkanPipelineBuilder& SetPipelineType(PipelineType type);
		VulkanPipelineBuilder& LayoutAddDescriptorLayout(VkDescriptorSetLayout layout);
		VulkanPipelineBuilder& LayoutAddPushRange(VkPushConstantRange range);
		VulkanPipelineBuilder& AddShader(std::shared_ptr<VulkanShader> shader);

		VulkanPipelineBuilder& SetColorAttachmentFormats(const std::vector<VkFormat>& formats);
		VulkanPipelineBuilder& SetDepthFormat(VkFormat format);
		VulkanPipelineBuilder& SetTopology(VkPrimitiveTopology topology);
		VulkanPipelineBuilder& SetPolygonMode(VkPolygonMode mode);
		VulkanPipelineBuilder& SetCullMode(VkCullModeFlags cullMode);
		VulkanPipelineBuilder& SetFrontFace(VkFrontFace frontFace);
		std::shared_ptr<VulkanPipeline> Build();

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

		std::vector<VkFormat>							m_ColorFormats; 
		VkFormat										m_DepthFormat{ VK_FORMAT_UNDEFINED };
		VkPrimitiveTopology								m_Topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
		VkPolygonMode									m_PolygonMode{ VK_POLYGON_MODE_FILL };
		VkCullModeFlags									m_CullMode{ VK_CULL_MODE_BACK_BIT };
		VkFrontFace										m_FrontFace{ VK_FRONT_FACE_CLOCKWISE };
	};

	class VulkanPipeline
	{
	public:
		VulkanPipeline(VkPipeline pipeline, VkPipelineLayout pipelineLayout);
		~VulkanPipeline();

		VkPipeline GetRaw() const { return m_Pipeline; }
		VkPipelineLayout GetLayout() const { return m_PipelineLayout; }

	private:
		VkPipeline m_Pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
	};

}