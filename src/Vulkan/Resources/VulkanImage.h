#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace tiny_vulkan {

	struct ImageSyncState
	{
		VkPipelineStageFlags2	lastStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		VkAccessFlags2			lastAccess = VK_ACCESS_2_NONE;
		VkImageLayout			lastLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	class VulkanImage
	{
	public:
		explicit VulkanImage(VkImage image, VkImageView view, VkFormat format, VkExtent3D extent, VmaAllocation allocation);

		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

		[[nodiscard]] VkImage		GetRaw()		const { return m_Image; }
		[[nodiscard]] VkImageView	GetView()		const { return m_View; }
		[[nodiscard]] VkFormat		GetFormat()		const { return m_Format; }
		[[nodiscard]] VkExtent3D	GetExtent()		const { return m_Extent; }
		[[nodiscard]] VmaAllocation	GetAllocation() const { return m_Allocation; }

		[[nodiscard]] const ImageSyncState& GetSyncState() const { return m_SyncState; }

		inline void SetSyncState(VkPipelineStageFlags2 newStage, VkAccessFlags2 newAccess, VkImageLayout newLayout)
		{
			m_SyncState = { newStage, newAccess, newLayout };
		}

	private:
		VkImage			m_Image{ VK_NULL_HANDLE };
		VkImageView		m_View{ VK_NULL_HANDLE };
		VkFormat		m_Format{ VK_FORMAT_UNDEFINED };
		VkExtent3D		m_Extent{ 0, 0, 0 };
		VmaAllocation	m_Allocation{ VK_NULL_HANDLE };
		ImageSyncState	m_SyncState;
	};

}