#include "VulkanBuffer.h"
#include "VulkanCore.h"
#include "LifetimeManager.h"

namespace tiny_vulkan {

	VulkanBuffer::VulkanBuffer(VkBuffer buffer, VmaAllocation allocation, VmaAllocationInfo allocationInfo)
		: m_Buffer(buffer)
		, m_Allocation(allocation)
		, m_VmaAllocationInfo(allocationInfo)
	{

	}

	VulkanBufferBuilder& VulkanBufferBuilder::SetAllocationSize(size_t allocSize)
	{
		m_AllocSize = allocSize;
		return *this;
	}

	VulkanBufferBuilder& VulkanBufferBuilder::SetUsageMask(VkBufferUsageFlags usageMask)
	{
		m_UsageMask = usageMask;
		return *this;
	}

	VulkanBufferBuilder& VulkanBufferBuilder::SetAllocationPlace(VmaMemoryUsage memoryUsagePlace)
	{
		m_MemoryUsagePlace = memoryUsagePlace;
		return *this;
	}

	std::shared_ptr<VulkanBuffer> VulkanBufferBuilder::Build()
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.flags = 0;
		bufferInfo.size = m_AllocSize;
		bufferInfo.usage = m_UsageMask;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = nullptr;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = m_MemoryUsagePlace;

		if (m_MemoryUsagePlace == VMA_MEMORY_USAGE_CPU_TO_GPU || m_MemoryUsagePlace == VMA_MEMORY_USAGE_CPU_ONLY) 
		{
			allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}

		auto allocator = VulkanCore::GetVmaAllocator();
		VkBuffer buffer{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		VmaAllocationInfo allocationInfo;

		CHECK_VK_RES(vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo));

		return std::make_shared<VulkanBuffer>(buffer, allocation, allocationInfo);
	}

}