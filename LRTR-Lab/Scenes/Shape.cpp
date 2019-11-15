#include "Shape.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

auto LRTR::Shape::components() const -> const Group<std::type_index, std::shared_ptr<Component>>& 
{
	return mComponents;
}

auto LRTR::Shape::typeName() const noexcept -> std::string
{
	return "Shape";
}

auto LRTR::Shape::typeIndex() const noexcept -> std::type_index
{
	return typeid(Shape);
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

	if (ImGui::TreeNode("Info")) {

		ImGui::Columns(2, "Info");

		ImGui::Separator();
		ImGui::Text("TypeInfo"); ImGui::NextColumn();
		ImGui::Text(typeName().c_str()); ImGui::NextColumn();
		ImGui::Columns(1);
		ImGui::Separator();

		ImGui::TreePop();
	}
	
	for (auto component : orderComponents) {
		if (ImGui::TreeNode(mComponents[component.first]->typeName().c_str())) {

			mComponents[component.first]->onProperty();
			
			ImGui::TreePop();
		}
	}
}