#include "tinyFilesystem.h"
#include "LogSystem.h"

namespace tiny_vulkan {

	std::string tinyFilesystem::ReadFile(std::filesystem::path path)
	{
		std::ifstream file(path, std::ios::in | std::ios::beg);
		if (!file.is_open())
		{
			LOG_ERROR(fmt::runtime("Failed to open file: {}"), path.string());
			return {};
		}

		std::string result;
		std::string temp;

		while (std::getline(file, temp))
		{
			result += temp + '\n';
		}

		return result;
	}

	std::vector<char> tinyFilesystem::ReadFileBin(std::filesystem::path path)
	{
		std::ifstream file(path, std::ios::in | std::ios::ate | std::ios::binary);
		if (file.is_open())
		{
			std::vector<char> bytes;

			std::size_t fileSize = file.tellg();
			bytes.resize(fileSize);
			file.seekg(0);

			file.read(bytes.data(), fileSize);

			return bytes;
		}
		else
		{
			LOG_ERROR(fmt::runtime("Failed to open file: {}"), path.string());
			return {};
		}
	}

}