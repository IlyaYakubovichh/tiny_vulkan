#pragma once

#include "VulkanImage.h"
#include <vulkan/vulkan.h>
#include <memory>

namespace tiny_vulkan {

	class VulkanTransfer
	{
	public:
		VulkanTransfer() = default;
		~VulkanTransfer() = default;
		
		static void Blit(
			VkCommandBuffer cmdBuffer,
			std::shared_ptr<VulkanImage> srcImage, 
			std::shared_ptr<VulkanImage> dstImage,
			VkExtent3D srcExtent,
			VkExtent3D dstExtent
		);
	};

}