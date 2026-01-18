#include "VulkanSync.h"

namespace tiny_vulkan {

	void VulkanSync::InsertImageMemoryBarrier(
		VkCommandBuffer cmdBuffer,
		std::shared_ptr<VulkanImage> image,
		VkPipelineStageFlags2 dstStage,
		VkAccessFlags2 dstAccess,
		VkImageLayout newLayout)
	{
		auto& syncState = image->GetSyncState();

		VkImageMemoryBarrier2 imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		imageBarrier.pNext = nullptr;
		imageBarrier.srcStageMask = syncState.lastStage;
		imageBarrier.srcAccessMask = syncState.lastAccess;
		imageBarrier.dstStageMask = dstStage;
		imageBarrier.dstAccessMask = dstAccess;
		imageBarrier.oldLayout = syncState.lastLayout;
		imageBarrier.newLayout = newLayout;

		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange.baseArrayLayer = 0;
		imageBarrier.subresourceRange.baseMipLevel = 0;
		imageBarrier.subresourceRange.layerCount = 1;
		imageBarrier.subresourceRange.levelCount = 1;
		imageBarrier.image = image->GetRaw();

		VkDependencyInfo depInfo = {};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;
		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &imageBarrier;

		vkCmdPipelineBarrier2(cmdBuffer, &depInfo);

		image->SetSyncState(dstStage, dstAccess, newLayout);
	}

}