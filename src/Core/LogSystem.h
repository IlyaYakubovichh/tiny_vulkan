#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace tiny_vulkan {

	using sink = spdlog::sinks::stdout_color_sink_mt;
	using logger = spdlog::logger;

	class LogSystem
	{
	public:
		static std::shared_ptr<sink>	vulkanSink;
		static std::shared_ptr<logger>	vulkanLogger;

		static void Initialize();

		template<typename... Args> static void LogTrace(Args&&... args) { vulkanLogger->trace(std::forward<Args>(args)...); }
		template<typename... Args> static void LogDebug(Args&&... args) { vulkanLogger->debug(std::forward<Args>(args)...); }
		template<typename... Args> static void LogInfo(Args&&... args) { vulkanLogger->info(std::forward<Args>(args)...); }
		template<typename... Args> static void LogWarn(Args&&... args) { vulkanLogger->warn(std::forward<Args>(args)...); }
		template<typename... Args> static void LogError(Args&&... args) { vulkanLogger->error(std::forward<Args>(args)...); }
		template<typename... Args> static void LogCritical(Args&&... args) { vulkanLogger->critical(std::forward<Args>(args)...); }
	};

}

#define LOG_TRACE(...)			  tiny_vulkan::LogSystem::LogTrace(__VA_ARGS__)
#define LOG_DEBUG(...)			  tiny_vulkan::LogSystem::LogDebug(__VA_ARGS__)
#define LOG_INFO(...)			  tiny_vulkan::LogSystem::LogInfo(__VA_ARGS__)
#define LOG_WARN(...)			  tiny_vulkan::LogSystem::LogWarn(__VA_ARGS__)
#define LOG_ERROR(...)			  tiny_vulkan::LogSystem::LogError(__VA_ARGS__)
#define LOG_CRITICAL(...)		  tiny_vulkan::LogSystem::LogCritical(__VA_ARGS__)