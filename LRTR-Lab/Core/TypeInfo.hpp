#pragma once

#include <typeindex>
#include <string>
#include <atomic>

namespace LRTR {

	using Identity = unsigned long long;
	
	class TypeInfo {
	public:
		TypeInfo() : mIdentity(++mGlobalIdentity) {}

		TypeInfo(const TypeInfo&) : mIdentity(++mGlobalIdentity) {}

		TypeInfo(TypeInfo&& other) noexcept : mIdentity(other.mIdentity) { other.mIdentity = 0; }
		
		virtual ~TypeInfo() { mIdentity = 0; }

		auto identity() const noexcept -> Identity { return mIdentity; }
		
		virtual auto typeName() const noexcept -> std::string = 0;

		virtual auto typeIndex() const noexcept -> std::type_index = 0;
	private:
		static inline std::atomic<Identity> mGlobalIdentity = 0;
		
		Identity mIdentity = 0;
	};
	
}