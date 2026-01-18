#pragma once

#include <functional>

namespace tiny_vulkan {

	class LifetimeManager
	{
	public:
		LifetimeManager() = default;
		~LifetimeManager() = default;

		template<typename F, typename... Args>
		static void PushFunction(F& function, Args&&... args)
		{
			m_Deleters.push_back([=]() -> void
				{
					function(args...);
				});
		}

		static void ExecuteAll()
		{
			for (auto it = m_Deleters.rbegin(); it != m_Deleters.rend(); ++it)
			{
				(*it)();
			}
			m_Deleters.clear();
		}

	private:
		static std::vector<std::function<void()>> m_Deleters;
	};

}