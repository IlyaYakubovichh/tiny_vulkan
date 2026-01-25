#include "VulkanShader.h"
#include "Filesystem.h"
#include "LifetimeManager.h"
#include "LogSystem.h"

#include <shaderc/shaderc.hpp>
#include <fstream>
#include <iostream>

namespace tiny_vulkan {

	// ==============================================================================
	// Helpers (Internal linkage)
	// ==============================================================================
	namespace {

		shaderc_shader_kind GetShadercKind(const std::filesystem::path& path)
		{
			auto ext = path.extension().string();
			if (ext == ".vert") return shaderc_vertex_shader;
			if (ext == ".frag") return shaderc_fragment_shader;
			if (ext == ".comp") return shaderc_compute_shader;
			if (ext == ".geom") return shaderc_geometry_shader;
			if (ext == ".tesc") return shaderc_tess_control_shader;
			if (ext == ".tese") return shaderc_tess_evaluation_shader;
			if (ext == ".mesh") return shaderc_mesh_shader;
			if (ext == ".task") return shaderc_task_shader;
			return shaderc_glsl_infer_from_source;
		}

		VkShaderStageFlagBits GetVkShaderStage(const std::filesystem::path& path)
		{
			auto ext = path.extension().string();
			if (ext == ".vert") return VK_SHADER_STAGE_VERTEX_BIT;
			if (ext == ".frag") return VK_SHADER_STAGE_FRAGMENT_BIT;
			if (ext == ".comp") return VK_SHADER_STAGE_COMPUTE_BIT;
			if (ext == ".geom") return VK_SHADER_STAGE_GEOMETRY_BIT;
			if (ext == ".tesc") return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			if (ext == ".tese") return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			if (ext == ".mesh") return VK_SHADER_STAGE_MESH_BIT_EXT;
			if (ext == ".task") return VK_SHADER_STAGE_TASK_BIT_EXT;
			return VK_SHADER_STAGE_ALL;
		}
	}

	// ==============================================================================
	// VulkanShader Implementation
	// ==============================================================================
	VulkanShader::VulkanShader(VkShaderModule module, VkShaderStageFlagBits stage, std::vector<uint32_t>&& spirv, const std::filesystem::path& path)
		: m_ShaderModule(module)
		, m_Stage(stage)
		, m_SPIRV(std::move(spirv))
		, m_ShaderPath(path)
	{

	}

	std::shared_ptr<VulkanShader> VulkanShader::Create(VkDevice device, const std::filesystem::path& shaderPath)
	{
		if (!std::filesystem::exists(shaderPath)) 
		{
			LOG_ERROR(fmt::runtime("Shader file not found: {}"), shaderPath.string());
			return nullptr;
		}

		std::vector<uint32_t> spirv;
		auto cachePath = GetCachedPath(shaderPath);

		// Try to load a cache( .spv ).
		if (CheckCacheValidity(shaderPath, cachePath))
		{
			LoadFromCache(shaderPath, spirv);
		}
		else
		{
			// Compile.
			if (!CompileToSPIRV(shaderPath, spirv)) return nullptr;
			SaveToCache(cachePath, spirv);
		}

		if (spirv.empty()) return nullptr;

		// Create shader module.
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv.size() * sizeof(uint32_t);
		createInfo.pCode = spirv.data();

		VkShaderModule module{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateShaderModule(device, &createInfo, nullptr, &module));

		LifetimeManager::PushFunction(vkDestroyShaderModule, device, module, nullptr);

		return std::make_shared<VulkanShader>(module, GetVkShaderStage(shaderPath), std::move(spirv), shaderPath);
	}

	// ==============================================================================
	// Cache & Compilation Logic
	// ==============================================================================
	std::filesystem::path VulkanShader::GetCacheDir()
	{
		auto path = std::filesystem::current_path() / "Cache" / "Shaders";
		if (!std::filesystem::exists(path))
		{
			std::filesystem::create_directories(path);
		}
		return path;
	}

	std::filesystem::path VulkanShader::GetCachedPath(const std::filesystem::path& sourcePath)
	{
		return GetCacheDir() / (sourcePath.stem().string() + ".spv");
	}

	bool VulkanShader::CheckCacheValidity(const std::filesystem::path& sourcePath, const std::filesystem::path& cachePath)
	{
		if (!std::filesystem::exists(cachePath)) return false;
		auto sourceTime = std::filesystem::last_write_time(sourcePath);
		auto cacheTime = std::filesystem::last_write_time(cachePath);
		return cacheTime > sourceTime;
	}

	bool VulkanShader::CompileToSPIRV(const std::filesystem::path& path, std::vector<uint32_t>& outSpirv)
	{
		auto glslSource = IO::ReadFile(path).value();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
			glslSource.c_str(),
			glslSource.size(),
			GetShadercKind(path),
			path.string().c_str(),
			"main",
			options
		);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR(fmt::runtime("Shader compilation failed ({}):\n{}"), path.string(), result.GetErrorMessage());
			return false;
		}

		outSpirv.assign(result.begin(), result.end());
		return true;
	}

	void VulkanShader::LoadFromCache(const std::filesystem::path& cachePath, std::vector<uint32_t>& outSpirv)
	{
		outSpirv = std::move(IO::ReadFileBin(cachePath).value());
	}

	void VulkanShader::SaveToCache(const std::filesystem::path& cachePath, const std::vector<uint32_t>& spirv)
	{
		std::ofstream file(cachePath, std::ios::binary);
		if (file.is_open()) 
		{
			file.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
		}
	}

}