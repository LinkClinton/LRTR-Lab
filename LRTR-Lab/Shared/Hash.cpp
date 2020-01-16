#include "Hash.hpp"

#include "../Extensions/Cryptopp/Cryptopp.hpp"

auto LRTR::Hash::sha256(const std::string& string) -> std::string
{
	return Cryptopp::sha256(string);
}
