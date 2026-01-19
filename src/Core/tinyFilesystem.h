#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace tiny_vulkan {

	class tinyFilesystem
	{
	public:
		tinyFilesystem() = default;
		~tinyFilesystem() = default;

		static std::string ReadFile(std::filesystem::path path);
		static std::vector<char> ReadFileBin(std::filesystem::path path);
	};

}