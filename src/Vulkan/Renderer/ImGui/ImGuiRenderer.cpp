#include "ImGui/ImGuiRenderer.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

namespace tiny_vulkan {

	static void CheckImGui(VkResult res)
	{
		if (res != VK_SUCCESS)
		{
			LOG_CRITICAL(fmt::runtime("CRITICAL VULKAN ERROR: {0}"), string_VkResult(res));         
			abort();                                                                                
		}
	}

	ImGuiRenderer::ImGuiRenderer(std::shared_ptr<Window> window, std::shared_ptr<VulkanCore> core)
		: m_Core(core)
		, m_Window(window)
	{
		// ========================================================
		// Prepare
		// ========================================================
		auto device				= core->GetDevice();
		auto swapchain			= core->GetSwapchain();
		auto images				= swapchain->GetImages();
		auto imageCount			= images.size();
		VkFormat imageFormat	= images[0]->GetFormat();

		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		CHECK_VK_RES(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_Pool));

		// ========================================================
		// ImGui for glfw + vulkan
		// ========================================================
		// Prepare
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		style.Colors[ImGuiCol_PopupBg].w = 1.0f;
		style.Colors[ImGuiCol_TitleBgActive].w = 1.0f;

		// glfw	
		ImGui_ImplGlfw_InitForVulkan(window->GetRaw(), true);

		// vulkan
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.ApiVersion = VK_API_VERSION_1_4;
		initInfo.Instance = core->GetInstance();
		initInfo.PhysicalDevice = core->GetPhysicalDevice();
		initInfo.Device = core->GetDevice();
		initInfo.QueueFamily = core->GetGraphicsFamily();
		initInfo.Queue = core->GetGraphicsQueue();
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = m_Pool;
		initInfo.MinImageCount = imageCount;
		initInfo.ImageCount = imageCount;
		initInfo.Allocator = nullptr;
		initInfo.UseDynamicRendering = true;

		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &imageFormat;

		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = CheckImGui;
		ImGui_ImplVulkan_Init(&initInfo);

		// ========================================================
		// Cleanup
		// ========================================================
		LifetimeManager::PushFunction(vkDestroyDescriptorPool, device, m_Pool, nullptr);
		LifetimeManager::PushFunction(ImGui_ImplVulkan_Shutdown);
	}

	void ImGuiRenderer::CalculateInternal()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();
	}

	void ImGuiRenderer::OnUpdate(const ImGuiOnUpdatePackage& package)
	{
		CalculateInternal();

		VkRenderingAttachmentInfo attachmentInfo = {};
		attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		attachmentInfo.pNext = nullptr;
		attachmentInfo.imageView = package.imageView;
		attachmentInfo.imageLayout = package.imageLayout;
		attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = 0;
		renderingInfo.renderArea = VkRect2D{ {0,0}, {package.extent.width, package.extent.height} };
		renderingInfo.layerCount = 1;
		renderingInfo.viewMask = 0;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &attachmentInfo;
		renderingInfo.pDepthAttachment = nullptr;
		renderingInfo.pStencilAttachment = nullptr;

		vkCmdBeginRendering(package.cmdBuffer, &renderingInfo);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), package.cmdBuffer);

		vkCmdEndRendering(package.cmdBuffer);
	}

}