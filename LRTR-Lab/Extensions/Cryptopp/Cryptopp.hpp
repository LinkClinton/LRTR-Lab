#pragma once

#include <string>

namespace LRTR {

	namespace Cryptopp {

		auto sha256(const std::string& string) -> std::string;
		
	}
	
}