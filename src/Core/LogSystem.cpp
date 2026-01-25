#include "LogSystem.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace tiny_vulkan::LogSystem {

	namespace {
		std::shared_ptr<spdlog::logger> g_TinyLogger;
	}

	void Initialize()
	{
		auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		sink->set_pattern("%^[%T] %n: %v%$");

		g_TinyLogger = std::make_shared<spdlog::logger>("tinyLogger", sink);
		g_TinyLogger->set_level(spdlog::level::trace);
	}

	std::shared_ptr<spdlog::logger> GetLogger()
	{
		return g_TinyLogger;
	}

}