#pragma once

#include "VulkanImage.h"
#include <vulkan/vulkan.h>
#include <memory>

namespace tiny_vulkan {

	class VulkanSync
	{
	public:
		VulkanSync() = default;
		~VulkanSync() = default;

		static void InsertImageMemoryBarrier(
			VkCommandBuffer cmdBuffer,
			std::shared_ptr<VulkanImage> image,
			VkPipelineStageFlags2 dstStage,
			VkAccessFlags2 dstAccess,
			VkImageLayout newLayout
		);
	};

}