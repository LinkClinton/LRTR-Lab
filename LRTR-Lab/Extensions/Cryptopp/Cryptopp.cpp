#include "Cryptopp.hpp"

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

using namespace CryptoPP;

auto LRTR::Cryptopp::sha256(const std::string& string) -> std::string
{
	auto digest = std::string();

	SHA256 sha256;

	const StringSource src(string, true, new HashFilter(
		sha256, new HexEncoder(new StringSink(digest))));

	return digest;
}
