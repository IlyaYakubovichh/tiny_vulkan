#include "Filesystem.h"
#include "LogSystem.h"

namespace tiny_vulkan::IO {

	std::optional<std::string> ReadFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::in | std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			LOG_ERROR(fmt::runtime("Failed to open file: {}"), path.string());
			return std::nullopt;
		}

		const auto fileSize = file.tellg();
		if (fileSize < 0)
		{
			LOG_ERROR(fmt::runtime("Failed to get file size: {}"), path.string());
			return std::nullopt;
		}

		if (fileSize == 0)
		{
			return std::string{};
		}

		std::string result;
		result.resize(static_cast<size_t>(fileSize));

		file.seekg(0, std::ios::beg);

		file.read(result.data(), fileSize);

		return result;
	}

	std::optional<std::vector<uint32_t>> ReadFileBin(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			LOG_ERROR(fmt::runtime("Failed to open file: {}"), path.string());
			return std::nullopt;
		}

		const auto fileSize = file.tellg();
		if (fileSize < 0) 
		{
			LOG_ERROR(fmt::runtime("Incorrect file size: {}"), path.string());
			return std::nullopt;
		}

		if (fileSize % sizeof(uint32_t) != 0) 
		{
			LOG_ERROR(fmt::runtime("Incorrect fileSize, not divisible by sizeof(uint32_t)!"));
			return std::nullopt;
		}

		std::vector<uint32_t> buffer;
		buffer.resize((static_cast<size_t>(fileSize)) / sizeof(uint32_t));

		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

		return buffer;
	}

}