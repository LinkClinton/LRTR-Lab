#pragma once

#include <unordered_map>
#include <string>

namespace LRTR {

	template<typename Key, typename Value>
	using Group = std::unordered_map<Key, Value>;

	template<typename Value>
	using StringGroup = std::unordered_map<std::string, Value>;
	
}