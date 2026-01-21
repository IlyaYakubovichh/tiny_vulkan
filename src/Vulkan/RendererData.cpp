#include "RendererData.h"
#include "VulkanRenderer.h"
#include <filesystem>

namespace tiny_vulkan {

	void RendererData::Initialize()
	{
		CreateShaders();
		CreatePipeline();
	}


	void RendererData::CreateShaders()
	{
		std::filesystem::path wd = std::filesystem::current_path() / ".." / "src" / "EntryPoint" / "Assets";
		m_VertexShader = std::make_shared<VulkanShader>(wd / "triangleV.vert");
		m_FragmentShader = std::make_shared<VulkanShader>(wd / "triangleF.frag");
	}

	void RendererData::CreatePipeline()
	{
		std::vector<VkFormat> colorAttachmentFormats;
		colorAttachmentFormats.push_back(VulkanRenderer::GetCore()->GetRenderTarget()->GetFormat());

		m_GraphicsPipeline = VulkanPipelineBuilder()
			.SetPipelineType(PipelineType::GRAPHICS)
			.AddShader(m_VertexShader)
			.AddShader(m_FragmentShader)
			.SetColorAttachmentFormats(colorAttachmentFormats)
			.SetDepthFormat(VK_FORMAT_UNDEFINED)
			.SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetPolygonMode(VK_POLYGON_MODE_FILL)
			.SetCullMode(VK_CULL_MODE_BACK_BIT)
			.SetFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.Build();
	}

}