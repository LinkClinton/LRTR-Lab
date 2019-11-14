#pragma once

#include "../Shared/Accelerators/Group.hpp"
#include "Component.hpp"

#include <typeindex>
#include <memory>

namespace LRTR {

	class Shape : public Noncopyable, public Propertyable, public TypeInfo {
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

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
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

}
