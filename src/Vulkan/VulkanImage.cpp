#include "VulkanImage.h"

namespace tiny_vulkan {

	VulkanImage::VulkanImage(VkImage image, VkImageView view, VkFormat format, VkExtent3D extent)
		: m_Image(image), m_View(view), m_Format(format), m_Extent(extent)
	{

	}

	VulkanImage::~VulkanImage()
	{

	}

	void VulkanImage::SetSyncState(VkPipelineStageFlags2 newStage, VkAccessFlags2 newAccess, VkImageLayout newLayout)
	{
		m_SyncState.lastStage = newStage;
		m_SyncState.lastAccess = newAccess;
		m_SyncState.lastLayout = newLayout;
	}

}