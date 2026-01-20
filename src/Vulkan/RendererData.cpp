#include "RendererData.h"
#include "VulkanRenderer.h"
#include <filesystem>

namespace tiny_vulkan {

	int ComputeEffect::m_SelectedEffectIndex = 0;

	void RendererData::Initialize()
	{
		CreateDescriptors();
		CreatePipelines();
	}

	void RendererData::CreateDescriptors()
	{
		// ========================================================
		// Layout & Pool
		// ========================================================
		m_DescriptorPool = VulkanDescriptorPoolBuilder()
			.AddMaxSets(1)
			.AddRatio(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			.Build();

		m_SetLayout = VkDescriptorSetLayoutBuilder()
			.AddBinding(0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			.Build();

		m_DescriptorSet = m_DescriptorPool->AllocateSet(m_SetLayout);

		UpdateDescriptors();
	}

	void RendererData::UpdateDescriptors()
	{
		// ========================================================
		// Update
		// ========================================================
		auto renderTarget = VulkanRenderer::GetCore()->GetRenderTarget();

		WriteImagePackage imagePackage;
		imagePackage.imageView = renderTarget->GetView();
		imagePackage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imagePackage.dstBinding = 0;
		imagePackage.descriptorCount = 1;
		imagePackage.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		m_DescriptorSet->WriteImageDescriptor(imagePackage);
	}

	void RendererData::CreatePipelines()
	{
		// ========================================================
		// Shaders
		// ========================================================
		std::filesystem::path wd = std::filesystem::current_path() / ".." / "src" / "EntryPoint" / "Assets";
		auto gradientShader = std::make_shared<VulkanShader>(wd / "gradient.comp");
		auto skyShader = std::make_shared<VulkanShader>(wd / "sky.comp");

		// ========================================================
		// Pipeline Config
		// ========================================================
		VkPushConstantRange range = {};
		range.offset = 0;
		range.size = sizeof(EffectParams);
		range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		// Helper lambda to reduce boilerplate
		auto buildPipeline = [&](std::shared_ptr<VulkanShader> shader)
			{
			return VkPipelineBuilder()
				.SetPipelineType(PipelineType::COMPUTE)
				.AddShader(shader)
				.LayoutAddDescriptorLayout(m_SetLayout)
				.LayoutAddPushRange(range)
				.Build();
			};

		// ========================================================
		// Effects Setup
		// ========================================================
		ComputeEffect gradientEffect;
		gradientEffect.name = "Gradient Effect";
		gradientEffect.computePipeline = buildPipeline(gradientShader);
		gradientEffect.params.values[0] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		gradientEffect.params.values[1] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

		ComputeEffect skyEffect;
		skyEffect.name = "Sky Effect";
		skyEffect.computePipeline = buildPipeline(skyShader);
		skyEffect.params.values[0] = glm::vec4(0.1f, 0.2f, 0.4f, 0.97f);

		m_ComputeEffects.push_back(gradientEffect);
		m_ComputeEffects.push_back(skyEffect);
	}

}