#include "VulkanImage.h"

namespace tiny_vulkan {

	VulkanImage::VulkanImage(VkImage image, VkImageView view, VkFormat format, VkExtent3D extent, VmaAllocation allocation)
		: m_Image(image),
		m_View(view),
		m_Format(format),
		m_Extent(extent),
		m_Allocation(allocation)
	{

	}

}