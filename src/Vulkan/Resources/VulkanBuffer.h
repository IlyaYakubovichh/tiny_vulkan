#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace tiny_vulkan {

	class VulkanBuffer
	{
	public:
		explicit VulkanBuffer(VkBuffer buffer, VmaAllocation allocation, VmaAllocationInfo allocationInfo);
		~VulkanBuffer() = default;

		[[nodiscard]] VkBuffer			GetRaw()			const { return m_Buffer; }
		[[nodiscard]] VmaAllocation		GetAllocation()		const { return m_Allocation; }
		[[nodiscard]] VmaAllocationInfo GetAllocationInfo() const { return m_VmaAllocationInfo; }

	private:
		VkBuffer			m_Buffer{ VK_NULL_HANDLE };
		VmaAllocation		m_Allocation{ VK_NULL_HANDLE };
		VmaAllocationInfo	m_VmaAllocationInfo{};
	};

	class VulkanBufferBuilder
	{
	public:
		explicit VulkanBufferBuilder() = default;
		~VulkanBufferBuilder() = default;

		[[nodiscard]] VulkanBufferBuilder& SetAllocationSize(size_t allocSize);
		[[nodiscard]] VulkanBufferBuilder& SetUsageMask(VkBufferUsageFlags usageMask);
		[[nodiscard]] VulkanBufferBuilder& SetAllocationPlace(VmaMemoryUsage memoryUsagePlace);
		[[nodiscard]] std::shared_ptr<VulkanBuffer> Build();

	private:
		size_t				m_AllocSize{ 0 };
		VkBufferUsageFlags	m_UsageMask{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
		VmaMemoryUsage		m_MemoryUsagePlace{ VMA_MEMORY_USAGE_UNKNOWN };
	};

}