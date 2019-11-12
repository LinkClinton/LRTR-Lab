#pragma once

#include <unordered_map>
#include <string>
#include <map>

namespace LRTR {

	template<typename Key, typename Value>
	using Group = std::unordered_map<Key, Value>;

	template<typename Key, typename Value>
	using OrderGroup = std::map<Key, Value>;
	
	template<typename Value>
	using StringGroup = std::unordered_map<std::string, Value>;

	template<typename Value>
	using StringOrderGroup = std::map<std::string, Value>;
}