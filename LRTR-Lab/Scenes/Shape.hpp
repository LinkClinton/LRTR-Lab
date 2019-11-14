#pragma once

#include "../Shared/Accelerators/Group.hpp"
#include "Component.hpp"

#include <typeindex>
#include <memory>

namespace LRTR {

	class Shape : public Noncopyable, public Propertyable {
	public:
		Shape() = default;

		~Shape() = default;

		template<typename TComponent>
		void addComponent(const std::shared_ptr<TComponent>& component);

		template<typename TComponent>
		void removeComponent();

		template<typename TComponent>
		auto component() const -> std::shared_ptr<TComponent>;

		auto components() const -> const Group<std::type_index, std::shared_ptr<Component>>&;

		template<typename TShape>
		static auto name() -> std::string;

		static auto name(const std::type_index& index) -> std::string;

		template<typename TShape>
		static void rename(const std::string& name);
	protected:
		void onProperty() override;
	private:
		friend class Runtime;
		
		static inline Group<std::type_index, std::string> mTypeName;

		static void initialize();
		
		Group<std::type_index, std::shared_ptr<Component>> mComponents;
		Group<std::type_index, size_t> mComponentsIndex;
	};

	template<typename Type>
	using IsShape = std::is_base_of<Shape, Type>;

	template <typename TComponent>
	void Shape::addComponent(const std::shared_ptr<TComponent>& component)
	{
		static_assert(IsComponent<TComponent>::value, "The Component should be based of Component.");
		
		static size_t order = 0;

		mComponents.insert({ typeid(TComponent), component });
		mComponentsIndex.insert({ typeid(TComponent),  order++ });
	}

	template <typename TComponent>
	void Shape::removeComponent()
	{
		static_assert(IsComponent<TComponent>::value, "The Component should be based of Component.");

		mComponents.erase(typeid(TComponent));
		mComponentsIndex.erase(typeid(TComponent));
	}

	template <typename TComponent>
	auto Shape::component() const -> std::shared_ptr<TComponent>
	{
		static_assert(IsComponent<TComponent>::value, "The Component should be based of Component.");

		return mComponents.at(typeid(TComponent));
	}

	template <typename TShape>
	auto Shape::name() -> std::string
	{
		static_assert(IsShape<TShape>::value, "The Type should be base of Shape.");

		if (mTypeName.find(typeid(TShape)) == mTypeName.end()) {
			LRTR_WARNING("the name of Shape is not set, we will return Unknown.");

			return "Unknown";
		}

		return mTypeName.at(typeid(TShape));
	}

	template <typename TShape>
	void Shape::rename(const std::string& name)
	{
		static_assert(IsShape<TShape>::value, "The Type should be base of Shape.");

		mTypeName[typeid(TShape)] = name;
	}

}
