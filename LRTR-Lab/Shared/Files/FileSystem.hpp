#pragma once

#include "../../Core/Logging.hpp"

#include <fstream>
#include <string>
#include <vector>

namespace LRTR {

	namespace FileSystem {

		template<typename ByteType>
		auto read(const std::string& fileName)->std::vector<ByteType> {
			static_assert(sizeof(ByteType) == 1);

			std::ifstream stream(fileName, std::ios::binary | std::ios::ate);

			LRTR_ERROR_IF(!stream.is_open(), "File @[{0}] is not exist.", fileName);

			const auto size = static_cast<size_t>(stream.tellg());

			auto code = std::vector<ByteType>(size);

			stream.seekg(0, std::ios::beg);
			stream.read(reinterpret_cast<char*>(code.data()), size);
			stream.close();

			return code;
		}

		template<typename ByteType>
		void write(const std::string& fileName, const std::vector<ByteType>& data) {
			static_assert(sizeof(ByteType) == 1);

			std::ofstream stream(fileName, std::ios::binary);

			stream.write(reinterpret_cast<const char*>(data.data()), data.size());
			stream.close();
		}

		auto read(const std::string& fileName)->std::string;

		void write(const std::string& fileName, const std::string& data);
	}
	
}