#pragma once

#include "../Shared/Accelerators/Group.hpp"

#include "../Core/Propertyable.hpp"
#include "../Core/Noncopyable.hpp"
#include "../Core/TypeInfo.hpp"
#include "../Core/Logging.hpp"

#include <type_traits>
#include <typeindex>
#include <string>

namespace LRTR {
	
	class Component : public Noncopyable, public Propertyable, public TypeInfo {
	public:
		Component() = default;

		~Component() = default;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	private:
		friend class Shape;
	};

	template<typename Type>
	using IsComponent = std::is_base_of<Component, Type>;
}
