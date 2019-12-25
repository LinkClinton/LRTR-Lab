#include "FileSystem.hpp"

auto LRTR::FileSystem::read(const std::string& fileName) -> std::string
{
	std::ifstream stream(fileName, std::ios::binary | std::ios::ate);

	LRTR_ERROR_IF(!stream.is_open(), "File @[{0}] is not exist.", fileName);

	const auto size = static_cast<size_t>(stream.tellg());

	std::string code; code.resize(size);

	stream.seekg(0, std::ios::beg);
	stream.read(static_cast<char*>(code.data()), size);
	stream.close();

	return code;
}

void LRTR::FileSystem::write(const std::string& fileName, const std::string& data)
{
	std::ofstream stream(fileName, std::ios::binary);

	stream.write(data.data(), data.size());
	stream.close();
}
