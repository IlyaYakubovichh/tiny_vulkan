#pragma once

#include <functional>

namespace tiny_vulkan::LifetimeManager {

	template<typename F, typename... Args>
	void PushFunction(F&& function, Args&&... args)
	{
		RegisterDeleter([func = std::forward<F>(function), ...args = std::forward<Args>(args)]() mutable
			{
				std::invoke(func, args...);
			});
	}

	void RegisterDeleter(std::function<void()>&& deleter);

	void ExecuteAll();
}