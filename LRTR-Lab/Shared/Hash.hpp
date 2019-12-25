#pragma once

#include <string>

namespace LRTR {

	namespace Hash {

		auto sha256(const std::string& string)->std::string;
		
	}
		
}