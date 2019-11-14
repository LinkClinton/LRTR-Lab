#pragma once

#include <typeindex>
#include <string>

namespace LRTR {

	class TypeInfo {
	public:
		TypeInfo() = default;

		virtual ~TypeInfo() = default;
		
		virtual auto typeName() const noexcept -> std::string = 0;

		virtual auto typeIndex() const noexcept -> std::type_index = 0;
	};
	
}