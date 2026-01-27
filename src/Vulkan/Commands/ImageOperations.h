#pragma once

#include "VulkanImage.h"

#include <memory>
#include <vulkan/vulkan.h>

namespace tiny_vulkan::ImageOperations {

	/**
	 * @brief Blits (copies with scaling/format conversion) from source to destination.
	 * Automatically handles Layout Transitions.
	 */
	void CmdBlit(
		VkCommandBuffer				 cmdBuffer,
		std::shared_ptr<VulkanImage> srcImage,
		std::shared_ptr<VulkanImage> dstImage,
		VkExtent3D					 srcExtent,
		VkExtent3D					 dstExtent
	);

}