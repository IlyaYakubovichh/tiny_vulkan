#include "LifetimeManager.h"
#include <vector>

namespace tiny_vulkan::LifetimeManager {

	namespace {
		// Internal linkage: accessible only within this translation unit.
		std::vector<std::function<void()>> g_Deleters;
	}

	void RegisterDeleter(std::function<void()>&& deleter)
	{
		g_Deleters.push_back(std::move(deleter));
	}

	void ExecuteAll()
	{
		// Iterate in reverse order (LIFO)
		for (auto it = g_Deleters.rbegin(); it != g_Deleters.rend(); ++it)
		{
			if (*it) {
				(*it)();
			}
		}
		g_Deleters.clear();
	}
}