#include "VulkanShader.h"
#include "VulkanRenderer.h"
#include "LifetimeManager.h"
#include "tinyFilesystem.h"
#include "VulkanUtils.h"
#include "LogSystem.h"

#include <shaderc/shaderc.hpp>

namespace tiny_vulkan {

	static shaderc_shader_kind GetShadercKind(const std::filesystem::path& shaderPath)
	{
		std::string shaderExt = shaderPath.extension().string();

		if (shaderExt == ".vert") return shaderc_vertex_shader;
		if (shaderExt == ".frag") return shaderc_fragment_shader;
		if (shaderExt == ".comp") return shaderc_compute_shader;
		if (shaderExt == ".geom") return shaderc_geometry_shader;

		LOG_WARN(fmt::runtime("Unsupported shader extension: {}"), shaderExt);
		return shaderc_glsl_infer_from_source;
	}

	VulkanShader::VulkanShader(std::filesystem::path shaderPath)
		: m_ShaderPath(shaderPath)
	{
		if (m_ShaderPath.empty())
		{
			LOG_CRITICAL("Empty shader path");
			return;
		}

		CompileOrLoad();
		CreateShaderModule();

		// Register cleanup
		auto core = VulkanRenderer::GetCore();
		VkDevice device = core->GetDevice();

		LifetimeManager::PushFunction(vkDestroyShaderModule, device, m_ShaderModule, nullptr);
	}

	std::filesystem::path VulkanShader::GetShaderCacheDir() const
	{
		return std::filesystem::current_path() / "Cache" / "Shaders";
	}

	std::filesystem::path VulkanShader::GetShaderCachedPath() const
	{
		return GetShaderCacheDir() / (m_ShaderPath.stem().string() + ".spv");
	}

	bool VulkanShader::IsShaderCacheValid() const
	{
		auto cachePath = GetShaderCachedPath();
		if (!std::filesystem::exists(cachePath)) return false;

		auto sourceTime = std::filesystem::last_write_time(m_ShaderPath);
		auto cacheTime = std::filesystem::last_write_time(cachePath);

		return cacheTime > sourceTime;
	}

	void VulkanShader::EnsureShaderCacheDirExists()
	{
		std::filesystem::create_directories(GetShaderCacheDir());
	}

	bool VulkanShader::CompileOrLoad()
	{
		EnsureShaderCacheDirExists();

		auto cachedPath = GetShaderCachedPath();

		// Load
		if (IsShaderCacheValid())
		{
			LOG_DEBUG(fmt::runtime("Loading cached SPIR-V: {}"), cachedPath.string());
			if (LoadFromCache()) return true;
			LOG_WARN("Failed to load cache, recompiling...");
		}

		// Compile
		LOG_DEBUG(fmt::runtime("Compiling shader: {}"), m_ShaderPath.string());

		std::string source = tinyFilesystem::ReadFile(m_ShaderPath);
		if (source.empty())
		{
			LOG_CRITICAL(fmt::runtime("Failed to read shader source: {}"), m_ShaderPath.string());
			return false;
		}

		return CompileAndCache(source);
	}

	bool VulkanShader::CompileAndCache(const std::string& source)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
			source.c_str(),
			source.size(),
			GetShadercKind(m_ShaderPath),
			m_ShaderPath.string().c_str(),
			"main",
			options
		);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_CRITICAL(fmt::runtime("Shader compilation failed ({}):\n{}"), m_ShaderPath.string(), result.GetErrorMessage());
			return false;
		}

		m_SPIRV.assign(result.begin(), result.end());

		// Write cache
		std::ofstream cacheFile(GetShaderCachedPath(), std::ios::binary | std::ios::trunc);
		if (cacheFile.is_open())
		{
			cacheFile.write(reinterpret_cast<const char*>(m_SPIRV.data()), m_SPIRV.size() * sizeof(uint32_t));
			LOG_DEBUG(fmt::runtime("Cached SPIR-V to: {}"), GetShaderCachedPath().string());
		}

		return true;
	}

	bool VulkanShader::LoadFromCache()
	{
		std::ifstream cacheFile(GetShaderCachedPath(), std::ios::binary | std::ios::ate);
		if (!cacheFile.is_open()) return false;

		auto fileSize = static_cast<std::size_t>(cacheFile.tellg());
		if (fileSize <= 0 || fileSize % sizeof(uint32_t) != 0)
		{
			LOG_WARN(fmt::runtime("Invalid SPIR-V cache file size: {}"), fileSize);
			return false;
		}

		cacheFile.seekg(0, std::ios::beg);
		m_SPIRV.resize(fileSize / sizeof(uint32_t));
		cacheFile.read(reinterpret_cast<char*>(m_SPIRV.data()), fileSize);

		return !m_SPIRV.empty();
	}

	void VulkanShader::CreateShaderModule()
	{
		if (m_SPIRV.empty())
		{
			LOG_CRITICAL(fmt::runtime("No SPIR-V data: {}"), m_ShaderPath.string());
		}

		auto core = VulkanRenderer::GetCore();

		VkShaderModuleCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.codeSize = m_SPIRV.size() * sizeof(uint32_t),
			.pCode = m_SPIRV.data()
		};

		CHECK_VK_RES(vkCreateShaderModule(core->GetDevice(), &createInfo, nullptr, &m_ShaderModule));
		LOG_DEBUG(fmt::runtime("Created shader module: {}"), m_ShaderPath.string());
	}

}