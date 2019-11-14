#include "Component.hpp"

#include "Components/Transform.hpp"

#define LRTR_ADD_COMPONENT_NAME(Component) rename<Component>(#Component)

auto LRTR::Component::name(const std::type_index& index) -> std::string
{
	if (mTypeName.find(index) == mTypeName.end()) {
		LRTR_WARNING("The name of Component is not set or the type is not Component, we will return Unknown.");

		return "Unknown";
	}

	return mTypeName.at(index);
}

void LRTR::Component::initialize()
{
	LRTR_ADD_COMPONENT_NAME(Component);

	LRTR_ADD_COMPONENT_NAME(Transform);
}
