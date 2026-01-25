#pragma once

#include <vulkan/vulkan.h>
#include <filesystem>
#include <vector>
#include <memory>
#include <string>

namespace tiny_vulkan {

	class VulkanShader
	{
	public:
		VulkanShader(VkShaderModule module, VkShaderStageFlagBits stage, std::vector<uint32_t>&& spirv, const std::filesystem::path& path);
		~VulkanShader() = default;

		[[nodiscard]] static std::shared_ptr<VulkanShader> Create(VkDevice device, const std::filesystem::path& shaderPath);

		[[nodiscard]] VkShaderModule				GetRaw()	const { return m_ShaderModule; }
		[[nodiscard]] VkShaderStageFlagBits			GetStage()	const { return m_Stage; }
		[[nodiscard]] const std::vector<uint32_t>&	GetCode()	const { return m_SPIRV; }

	private:
		static std::filesystem::path GetCacheDir();
		static std::filesystem::path GetCachedPath(const std::filesystem::path& sourcePath);
		static bool CheckCacheValidity(const std::filesystem::path& sourcePath, const std::filesystem::path& cachePath);

		static bool CompileToSPIRV(const std::filesystem::path& path, std::vector<uint32_t>& outSpirv);
		static void LoadFromCache(const std::filesystem::path& cachePath, std::vector<uint32_t>& outSpirv);
		static void SaveToCache(const std::filesystem::path& cachePath, const std::vector<uint32_t>& spirv);

	private:
		VkShaderModule			m_ShaderModule{ VK_NULL_HANDLE };
		VkShaderStageFlagBits	m_Stage{ VK_SHADER_STAGE_ALL };
		std::filesystem::path	m_ShaderPath;
		std::vector<uint32_t>	m_SPIRV; 
	};

}