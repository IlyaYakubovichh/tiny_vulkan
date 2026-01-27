#pragma once

#include <vector>
#include <filesystem>
#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	class VulkanShader
	{
	public:
		VulkanShader(const std::filesystem::path& path);
		~VulkanShader() = default;

		[[nodiscard]] VkShaderModule				GetRaw()	const { return m_ShaderModule; }
		[[nodiscard]] VkShaderStageFlagBits			GetStage()	const { return m_Stage; }
		[[nodiscard]] const std::vector<uint32_t>&  GetCode()	const { return m_SPIRV; }

	private:
		[[nodiscard]]  static std::filesystem::path GetCacheDir();
		[[nodiscard]]  static std::filesystem::path GetCachedPath(const std::filesystem::path& sourcePath);
		[[nodiscard]]  static bool CheckCacheValidity(const std::filesystem::path& sourcePath, const std::filesystem::path& cachePath);

		[[nodiscard]]  static bool CompileToSPIRV(const std::filesystem::path& path, std::vector<uint32_t>& outSpirv);
		static void LoadFromCache(const std::filesystem::path& cachePath, std::vector<uint32_t>& outSpirv);
		static void SaveToCache(const std::filesystem::path& cachePath, const std::vector<uint32_t>& spirv);

	private:
		VkShaderModule			m_ShaderModule{ VK_NULL_HANDLE };
		VkShaderStageFlagBits	m_Stage{ VK_SHADER_STAGE_ALL };
		std::filesystem::path	m_ShaderPath;
		std::vector<uint32_t>	m_SPIRV;
	};

}