#pragma once

#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	struct ImageSyncState
	{
		VkPipelineStageFlags2 lastStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		VkAccessFlags2 lastAccess = VK_ACCESS_2_NONE;
		VkImageLayout lastLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	class VulkanImage
	{
	public:
		VulkanImage(VkImage image, VkImageView view, VkFormat format, VkExtent3D extent);
		~VulkanImage();

		auto GetRaw() const { return m_Image; }
		auto GetView() const { return m_View; }
		auto& GetSyncState() const { return m_SyncState; }

		void SetSyncState(VkPipelineStageFlags2 newStage, VkAccessFlags2 newAccess, VkImageLayout newLayout);

	private:
		VkImage m_Image{ VK_NULL_HANDLE };
		VkImageView m_View{ VK_NULL_HANDLE };
		VkFormat m_Format;
		VkExtent3D m_Extent;
		ImageSyncState m_SyncState;
	};

}