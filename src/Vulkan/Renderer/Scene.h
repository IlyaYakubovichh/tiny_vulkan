#pragma once

namespace tiny_vulkan {

	struct ScenePushConstants
	{

	};

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		// Required commands in order to draw a scene (renderer will call it).
		void Render();
	};

}