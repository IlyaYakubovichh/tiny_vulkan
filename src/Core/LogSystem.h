#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <utility>

namespace tiny_vulkan::LogSystem {

	void Initialize();

	std::shared_ptr<spdlog::logger> GetLogger();

	template<typename... Args>
	void LogTrace(Args&&... args) { GetLogger()->trace(std::forward<Args>(args)...); }

	template<typename... Args>
	void LogDebug(Args&&... args) { GetLogger()->debug(std::forward<Args>(args)...); }

	template<typename... Args>
	void LogInfo(Args&&... args) { GetLogger()->info(std::forward<Args>(args)...); }

	template<typename... Args>
	void LogWarn(Args&&... args) { GetLogger()->warn(std::forward<Args>(args)...); }

	template<typename... Args>
	void LogError(Args&&... args) { GetLogger()->error(std::forward<Args>(args)...); }

	template<typename... Args>
	void LogCritical(Args&&... args) { GetLogger()->critical(std::forward<Args>(args)...); }

}

#define LOG_TRACE(...)    tiny_vulkan::LogSystem::LogTrace(__VA_ARGS__)
#define LOG_DEBUG(...)    tiny_vulkan::LogSystem::LogDebug(__VA_ARGS__)
#define LOG_INFO(...)     tiny_vulkan::LogSystem::LogInfo(__VA_ARGS__)
#define LOG_WARN(...)     tiny_vulkan::LogSystem::LogWarn(__VA_ARGS__)
#define LOG_ERROR(...)    tiny_vulkan::LogSystem::LogError(__VA_ARGS__)
#define LOG_CRITICAL(...) tiny_vulkan::LogSystem::LogCritical(__VA_ARGS__)