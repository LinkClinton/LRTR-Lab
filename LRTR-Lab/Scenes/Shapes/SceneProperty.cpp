#include "SceneProperty.hpp"

#include "../Components/CoordinateSystem.hpp"
#include "../Components/CameraGroup.hpp"

LRTR::SceneProperty::SceneProperty()
{
	addComponent(std::make_shared<CoordinateSystem>());
	addComponent(std::make_shared<CameraGroup>());
}

auto LRTR::SceneProperty::typeName() const noexcept -> std::string
{
	return "Scene";
}

auto LRTR::SceneProperty::typeIndex() const noexcept -> std::type_index
{
	return typeid(SceneProperty);
}
