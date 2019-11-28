#include "Shape.hpp"

#include "Components/CollectionLabel.hpp"
#include "../Extensions/ImGui/ImGui.hpp"

LRTR::Shape::Shape()
{
	addComponent(std::make_shared<CollectionLabel>());
}

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

	static auto treeNodeFlags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Framed;
	
	if (ImGui::TreeNodeEx("Info", treeNodeFlags)) {
		ImGui::BeginPropertyTable("Info");
		ImGui::Property("TypeInfo", [&]() {ImGui::Text(typeName().c_str()); });
		ImGui::EndPropertyTable();

		ImGui::TreePop();
	}
	
	for (auto component : orderComponents) {
		if (ImGui::TreeNodeEx(mComponents[component.first]->typeName().c_str(), treeNodeFlags)) {

			mComponents[component.first]->onProperty();
			
			ImGui::TreePop();
		}
	}
}