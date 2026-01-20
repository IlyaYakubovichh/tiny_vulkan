#include "RendererData.h"
#include "VulkanRenderer.h"

namespace tiny_vulkan {

	void RendererData::Initialize()
	{
		// ========================================================
		// Push constants
		// ========================================================
		pushConstants.data1 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		pushConstants.data2 = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		pushConstants.data3 = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		pushConstants.data4 = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

		// ========================================================
		// Descriptors
		// ========================================================
		desriptorPool = VulkanDescriptorPoolBuilder()
			.AddMaxSets(1)
			.AddRatio(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			.Build();

		setLayout = VkDescriptorSetLayoutBuilder()
			.AddBinding(0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			.Build();

		descriptorSet = desriptorPool->AllocateSet(setLayout);

		WriteImagePackage imagePackage;
		imagePackage.imageView = VulkanRenderer::GetCore()->GetRenderTarget()->GetView();
		imagePackage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imagePackage.dstBinding = 0;
		imagePackage.descriptorCount = 1;
		imagePackage.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorSet->WriteImageDescriptor(imagePackage);

		// ========================================================
		// Shaders
		// ========================================================
		std::filesystem::path wd = std::filesystem::current_path() / ".." / "src" / "EntryPoint" / "Assets";
		std::filesystem::path computeShaderPath = wd / "Compute.comp";
		computeShader = std::make_shared<VulkanShader>(computeShaderPath);

		// ========================================================
		// Pipelines
		// ========================================================
		VkPushConstantRange range = {};
		range.offset = 0;
		range.size = sizeof(ComputePushConstants);
		range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		pipeline = VkPipelineBuilder()
			.SetPipelineType(PipelineType::COMPUTE)
			.AddShader(computeShader)
			.LayoutAddDescriptorLayout(setLayout)
			.LayoutAddPushRange(range)
			.Build();
	}

}