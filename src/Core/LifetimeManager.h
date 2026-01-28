#pragma once

#include <functional>

namespace tiny_vulkan::LifetimeManager {

	template<typename F, typename... Args>
	void PushFunction(F&& function, Args&&... args)
	{
		RegisterDeleter([func = std::forward<F>(function), ...args = std::forward<Args>(args)]()
			{
				std::invoke(func, args...);
			});
	}

	template<typename F, typename... Args>
	void ExecuteNow(F&& function, Args&&... args)
	{
		std::invoke(std::forward<F>(function), std::forward<Args>(args)...);
	}

	void RegisterDeleter(std::function<void()>&& deleter);

	void ExecuteAll();
}