#include "SceneProperty.hpp"

#include "../Components/LinesMesh/CoordinateSystem.hpp"
#include "../Components/LinesMesh/LinesGrid.hpp"
#include "../Components/TransformWrap.hpp"
#include "../Components/CameraGroup.hpp"

LRTR::SceneProperty::SceneProperty()
{
	addComponent(std::make_shared<CoordinateSystem>());
	addComponent(std::make_shared<CameraGroup>());
	addComponent(std::make_shared<LinesGrid>(RectangleF(-5, -5, 5, 5), 10, 10,
		Vector3f(1, 0, 0), Vector3f(0, 1, 0),
		Vector3f(0, 0, -0.001f)));
}

auto LRTR::SceneProperty::typeName() const noexcept -> std::string
{
	return "Scene";
}

auto LRTR::SceneProperty::typeIndex() const noexcept -> std::type_index
{
	return typeid(SceneProperty);
}
