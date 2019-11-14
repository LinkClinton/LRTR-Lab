#include "Component.hpp"

auto LRTR::Component::typeName() const noexcept -> std::string
{
	return "Component";
}

auto LRTR::Component::typeIndex() const noexcept -> std::type_index
{
	return typeid(Component);
}
