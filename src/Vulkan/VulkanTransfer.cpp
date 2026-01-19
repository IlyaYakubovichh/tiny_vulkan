#include "VulkanTransfer.h"
#include "VulkanSync.h"

namespace tiny_vulkan {

	void VulkanTransfer::Blit(
		VkCommandBuffer cmdBuffer, 
		std::shared_ptr<VulkanImage> srcImage, 
		std::shared_ptr<VulkanImage> dstImage,
		VkExtent3D srcExtent,
		VkExtent3D dstExtent)
	{
		// ========================================================
		// Barriers 
		// ========================================================
		// SrcImage barrier
		VulkanSync::InsertImageMemoryBarrier(
			cmdBuffer, 
			srcImage, 
			VK_PIPELINE_STAGE_2_TRANSFER_BIT, 
			VK_ACCESS_2_TRANSFER_READ_BIT, 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		// DstImage barrier
		VulkanSync::InsertImageMemoryBarrier(
			cmdBuffer,
			dstImage,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// ========================================================
		// Blit
		// ========================================================
		VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

		blitRegion.srcOffsets[1].x = srcExtent.width;
		blitRegion.srcOffsets[1].y = srcExtent.height;
		blitRegion.srcOffsets[1].z = srcExtent.depth;

		blitRegion.dstOffsets[1].x = dstExtent.width;
		blitRegion.dstOffsets[1].y = dstExtent.height;
		blitRegion.dstOffsets[1].z = dstExtent.depth;

		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
		blitInfo.dstImage = dstImage->GetRaw();
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.srcImage = srcImage->GetRaw();
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		vkCmdBlitImage2(cmdBuffer, &blitInfo);

		// ========================================================
		// Update sync state
		// ========================================================
		srcImage->SetSyncState(VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		dstImage->SetSyncState(VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

}