#include "SceneProperty.hpp"

#include "../Components/CoordinateSystem.hpp"
#include "../Components/TransformWrap.hpp"
#include "../Components/CameraGroup.hpp"
#include "../Components/LinesMesh.hpp"

LRTR::SceneProperty::SceneProperty()
{
	addComponent(std::make_shared<CoordinateSystem>());
	addComponent(std::make_shared<CameraGroup>());
	addComponent(std::make_shared<LinesMesh>(
		std::vector<Line>({
			Line(Vector3f(0), Vector3f(1), ColorF(1,0,0,1)),
			Line(Vector3f(0), Vector3f(1), ColorF(0,1,0,1)),
		})));
}

auto LRTR::SceneProperty::typeName() const noexcept -> std::string
{
	return "Scene";
}

auto LRTR::SceneProperty::typeIndex() const noexcept -> std::type_index
{
	return typeid(SceneProperty);
}
