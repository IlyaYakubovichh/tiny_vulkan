#include "LogSystem.h"

namespace tiny_vulkan {

	std::shared_ptr<sink>	LogSystem::vulkanSink;
	std::shared_ptr<logger> LogSystem::vulkanLogger;

	void LogSystem::Initialize()
	{
		vulkanSink = std::make_shared<sink>();
		vulkanSink->set_pattern("%^[%T] %n: %v%$");
		vulkanLogger = std::make_shared<logger>("vulkanLogger", vulkanSink);
		vulkanLogger->set_level(spdlog::level::trace);
	}

}