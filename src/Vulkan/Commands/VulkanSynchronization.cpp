#include "VulkanSynchronization.h"

namespace tiny_vulkan::Synchronization {

	void CmdImageMemoryBarrier(
		VkCommandBuffer					cmdBuffer,
		std::shared_ptr<VulkanImage>	image,
		VkPipelineStageFlags2			dstStage,
		VkAccessFlags2					dstAccess,
		VkImageLayout					newLayout,
		VkImageAspectFlags				aspectMask)
	{
		auto& syncState = image->GetSyncState();

		VkImageMemoryBarrier2 imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		imageBarrier.pNext = nullptr;

		// Source: Current state of the image
		imageBarrier.srcStageMask = syncState.lastStage;
		imageBarrier.srcAccessMask = syncState.lastAccess;
		imageBarrier.oldLayout = syncState.lastLayout;

		// Destination: Target state
		imageBarrier.dstStageMask = dstStage;
		imageBarrier.dstAccessMask = dstAccess;
		imageBarrier.newLayout = newLayout;

		// Target Image
		imageBarrier.image = image->GetRaw();
		imageBarrier.subresourceRange.aspectMask = aspectMask;
		imageBarrier.subresourceRange.baseArrayLayer = 0;
		imageBarrier.subresourceRange.baseMipLevel = 0;
		imageBarrier.subresourceRange.layerCount = 1;
		imageBarrier.subresourceRange.levelCount = 1;

		VkDependencyInfo depInfo = {};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &imageBarrier;

		vkCmdPipelineBarrier2(cmdBuffer, &depInfo);

		// Update the internal state tracker so the next barrier knows where to start
		image->SetSyncState(dstStage, dstAccess, newLayout);
	}

}