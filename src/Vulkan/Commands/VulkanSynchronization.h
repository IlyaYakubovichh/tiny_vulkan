#pragma once

#include "VulkanImage.h"

#include <memory>
#include <vulkan/vulkan.h>

namespace tiny_vulkan::Synchronization {

	/**
	 * @brief Records a pipeline barrier to transition image layout and sync access.
	 * Updates the internal SyncState of the VulkanImage.
	 */
	void CmdImageMemoryBarrier(
		VkCommandBuffer					cmdBuffer,
		std::shared_ptr<VulkanImage>	image,
		VkPipelineStageFlags2			dstStage,
		VkAccessFlags2					dstAccess,
		VkImageLayout					newLayout,
		VkImageAspectFlags				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
	);

}