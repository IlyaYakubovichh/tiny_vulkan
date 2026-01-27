#include "ImageOperations.h"
#include "VulkanSynchronization.h"

namespace tiny_vulkan::ImageOperations {

	void CmdBlit(
		VkCommandBuffer				 cmdBuffer,
		std::shared_ptr<VulkanImage> srcImage,
		std::shared_ptr<VulkanImage> dstImage,
		VkExtent3D					 srcExtent,
		VkExtent3D					 dstExtent)
	{
		// ========================================================
		// Transition layouts for Transfer
		// ========================================================
		// Prepare Source
		Synchronization::CmdImageMemoryBarrier(
			cmdBuffer,
			srcImage,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_ACCESS_2_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		);

		// Prepare Destination
		Synchronization::CmdImageMemoryBarrier(
			cmdBuffer,
			dstImage,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		// ========================================================
		// Record Blit Command
		// ========================================================
		VkImageBlit2 blitRegion = {};
		blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

		// Source Region
		blitRegion.srcOffsets[1] = { static_cast<int32_t>(srcExtent.width), static_cast<int32_t>(srcExtent.height), static_cast<int32_t>(srcExtent.depth) };
		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.layerCount = 1;

		// Destination Region
		blitRegion.dstOffsets[1] = { static_cast<int32_t>(dstExtent.width), static_cast<int32_t>(dstExtent.height), static_cast<int32_t>(dstExtent.depth) };
		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.layerCount = 1;

		VkBlitImageInfo2 blitInfo = {};
		blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
		blitInfo.srcImage = srcImage->GetRaw();
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.dstImage = dstImage->GetRaw();
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		vkCmdBlitImage2(cmdBuffer, &blitInfo);
	}

}