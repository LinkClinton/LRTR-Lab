#include "Shape.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

#define LRTR_ADD_SHAPE_NAME(Shape) rename<Shape>(#Shape)

auto LRTR::Shape::components() const -> const Group<std::type_index, std::shared_ptr<Component>>& 
{
	return mComponents;
}

auto LRTR::Shape::name(const std::type_index& index) -> std::string
{
	if (mTypeName.find(index) == mTypeName.end()) {
		LRTR_WARNING("The name of Shape is not set or the type is not Shape, we will return Unknown.");

		return "Unknown";
	}

	return mTypeName.at(index);
}

void LRTR::Shape::onProperty()
{	
	auto orderComponents = std::vector<std::pair<std::type_index, size_t>>(
		mComponentsIndex.begin(), mComponentsIndex.end());

	std::sort(orderComponents.begin(), orderComponents.end(),
		[](
			const std::pair<std::type_index, size_t>& first,
			const std::pair<std::type_index, size_t>& second)
		{
			return  first.second < second.second;
		});

	for (auto component : orderComponents) {
		if (ImGui::TreeNode(Component::name(component.first).c_str())) {

			mComponents[component.first]->onProperty();
			
			ImGui::TreePop();
		}
	}
}

void LRTR::Shape::initialize()
{
	LRTR_ADD_SHAPE_NAME(Shape);
}
