#include "VulkanRenderer.h"

int main()
{
    tiny_vulkan::VulkanRenderer::Initialize();
    tiny_vulkan::VulkanRenderer::Run();
    tiny_vulkan::VulkanRenderer::Shutdown();

    return 0;
}