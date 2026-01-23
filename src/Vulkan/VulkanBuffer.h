#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace tiny_vulkan {

	class VulkanBuffer
	{
	public:
		VulkanBuffer(VkBuffer buffer, VmaAllocation allocation, VmaAllocationInfo allocationInfo);
		~VulkanBuffer() = default;

		auto GetRaw() const { return m_Buffer; }
		auto GetAllocation() const { return m_Allocation; }
		auto GetAllocationInfo() const { return m_VmaAllocationInfo; }

	private:
		VkBuffer m_Buffer{ VK_NULL_HANDLE };
		VmaAllocation m_Allocation{ VK_NULL_HANDLE };
		VmaAllocationInfo m_VmaAllocationInfo;
	};

	class VulkanBufferBuilder
	{
	public:
		VulkanBufferBuilder() = default;
		~VulkanBufferBuilder() = default;

		VulkanBufferBuilder& SetAllocationSize(size_t allocSize);
		VulkanBufferBuilder& SetUsageMask(VkBufferUsageFlags usageMask);
		VulkanBufferBuilder& SetAllocationPlace(VmaMemoryUsage memoryUsagePlace);
		std::shared_ptr<VulkanBuffer> Build();

	private:
		size_t m_AllocSize;
		VkBufferUsageFlags m_UsageMask;
		VmaMemoryUsage m_MemoryUsagePlace;
	};

}