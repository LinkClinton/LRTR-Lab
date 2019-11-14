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
	
	class Component : public Noncopyable, public Propertyable{
	public:
		Component() = default;

		~Component() = default;

		template<typename TComponent>
		static auto name() -> std::string;

		static auto name(const std::type_index& index) -> std::string;

		template<typename TComponent>
		static void rename(const std::string& name);
	private:
		friend class Shape;
		friend class Runtime;

		static inline Group<std::type_index, std::string> mTypeName;

		static void initialize();
	};

	template<typename Type>
	using IsComponent = std::is_base_of<Component, Type>;
	
	template <typename TComponent>
	auto Component::name() -> std::string
	{
		static_assert(IsComponent<TComponent>::value, "The Component should be based of Component.");

		if (mTypeName.find(typeid(TComponent)) == mTypeName.end()) {
			LRTR_WARNING("The name of Component is not set, we will return Unknown.");

			return "Unknown";
		}
		
		return mTypeName.at(typeid(Component));
	}

	template <typename TComponent>
	void Component::rename(const std::string& name)
	{
		static_assert(IsComponent<TComponent>::value, "The Component should be based of Component.");

		mTypeName[typeid(TComponent)] = name;
	}

}
