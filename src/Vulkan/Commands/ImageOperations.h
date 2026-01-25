#pragma once

#include "VulkanImage.h"
#include <vulkan/vulkan.h>

namespace tiny_vulkan::ImageOperations {

	/**
	 * @brief Blits (copies with scaling/format conversion) from source to destination.
	 * Automatically handles Layout Transitions.
	 */
	void CmdBlit(
		VkCommandBuffer cmdBuffer,
		VulkanImage* srcImage,
		VulkanImage* dstImage,
		VkExtent3D srcExtent,
		VkExtent3D dstExtent
	);

}