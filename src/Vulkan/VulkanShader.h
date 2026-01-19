#pragma once

#include <vulkan/vulkan.h>
#include <filesystem>
#include <vector>

namespace tiny_vulkan {

	class VulkanShader
	{
	public:
		VulkanShader(std::filesystem::path shaderPath);
		virtual ~VulkanShader() = default;

		VkShaderModule GetRaw() const { return m_ShaderModule; }

	private:
		std::filesystem::path GetShaderCacheDir()		const;
		std::filesystem::path GetShaderCachedPath()		const;
		bool IsShaderCacheValid()						const;
		void EnsureShaderCacheDirExists();

		bool CompileOrLoad();
		bool CompileAndCache(const std::string& source);
		bool LoadFromCache();

		void CreateShaderModule();

	private:
		VkShaderModule			m_ShaderModule{ VK_NULL_HANDLE };
		std::filesystem::path	m_ShaderPath;
		std::vector<uint32_t>	m_SPIRV;
	};


}