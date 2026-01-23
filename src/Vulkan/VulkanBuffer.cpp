#include "VulkanBuffer.h"
#include "VulkanRenderer.h"
#include "LifetimeManager.h"
#include "VulkanUtils.h"

namespace tiny_vulkan {

	// ========================================================
	// VulkanBuffer
	// ========================================================
	VulkanBuffer::VulkanBuffer(VkBuffer buffer, VmaAllocation allocation, VmaAllocationInfo allocationInfo)
		: m_Buffer(buffer)
		, m_Allocation(allocation)
		, m_VmaAllocationInfo(allocationInfo)
	{

	}

	// ========================================================
	// VulkanBuffer Builder
	// ========================================================
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
		auto allocator = VulkanRenderer::GetCore()->GetVmaAllocator();

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
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer buffer{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		VmaAllocationInfo allocationInfo; // pointer to the mapped memory will be stored here
		
		CHECK_VK_RES(vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo));

		LifetimeManager::PushFunction(vmaDestroyBuffer, allocator, buffer, allocation);

		return std::make_shared<VulkanBuffer>(buffer, allocation, allocationInfo);
	}

}