#include "LifetimeManager.h"

namespace tiny_vulkan {

	std::vector<std::function<void()>> LifetimeManager::m_Deleters;

}