#pragma once

#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	struct ImGuiOnUpdatePackage
	{
		VkCommandBuffer cmdBuffer;
		VkImageView		imageView;
		VkImageLayout	imageLayout;
		VkExtent3D		extent;
	};

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer() = delete;
		~ImGuiRenderer() = delete;

		static void Initialize();
		static void CalculateInternal();
		static void OnUpdate(const ImGuiOnUpdatePackage& package);

	private:
		static VkDescriptorPool m_Pool;
	};

}