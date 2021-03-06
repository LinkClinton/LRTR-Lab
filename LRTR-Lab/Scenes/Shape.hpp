#pragma once

#include "../Shared/Accelerators/Group.hpp"
#include "Component.hpp"

#include <typeindex>
#include <memory>

namespace LRTR {

	class Shape : public Noncopyable, public Propertyable, public TypeInfo {
	public:
		Shape();

		~Shape() = default;
		
		template<typename TComponent>
		void addComponent(const std::shared_ptr<TComponent>& component);

		template<typename TComponent>
		void setComponent(const std::shared_ptr<TComponent>& component);
		
		template<typename TComponent>
		void removeComponent();

		template<typename TComponent>
		auto component() const -> std::shared_ptr<TComponent>;

		template<typename TComponent>
		auto hasComponent() const -> bool;
		
		auto components() const -> const Group<std::type_index, std::shared_ptr<Component>>&;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		Group<std::type_index, std::shared_ptr<Component>> mComponents;
		Group<std::type_index, size_t> mComponentsIndex;

		size_t mOrder = 0;
	};

	template<typename Type>
	using IsShape = std::is_base_of<Shape, Type>;

	template <typename TComponent>
	void Shape::addComponent(const std::shared_ptr<TComponent>& component)
	{
		static_assert(IsComponent<TComponent>::value, "The Component should be based of Component.");
		
		mComponents.insert({ typeid(TComponent), component });
		mComponentsIndex.insert({ typeid(TComponent),  mOrder++ });
	}

	template <typename TComponent>
	void Shape::setComponent(const std::shared_ptr<TComponent>& component)
	{
		static_assert(IsComponent<TComponent>::value, "The Component should be based of Component.");

		if (!hasComponent<TComponent>()) addComponent(component);
		else mComponents[typeid(TComponent)] = component;
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

		return std::dynamic_pointer_cast<TComponent>(mComponents.at(typeid(TComponent)));
	}

	template <typename TComponent>
	auto Shape::hasComponent() const -> bool
	{
		static_assert(IsComponent<TComponent>::value, "The Component should be based of Component.");

		return mComponents.find(typeid(TComponent)) != mComponents.end();
	}

}
