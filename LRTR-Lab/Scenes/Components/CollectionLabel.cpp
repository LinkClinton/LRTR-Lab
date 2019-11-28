#include "CollectionLabel.hpp"

#include "../../Extensions/ImGui/ImGui.hpp"

LRTR::CollectionLabel::CollectionLabel(const std::string& label) :
	mLabel(label)
{
	
}

void LRTR::CollectionLabel::set(const std::string& label)
{
	mLabel = label;
}

auto LRTR::CollectionLabel::label() const noexcept -> std::string
{
	return mLabel;
}

auto LRTR::CollectionLabel::typeName() const noexcept -> std::string
{
	return "Collection";
}

auto LRTR::CollectionLabel::typeIndex() const noexcept -> std::type_index
{
	return typeid(CollectionLabel);
}

void LRTR::CollectionLabel::onProperty()
{
	static const size_t MaxLabel = 20;
	static char label[MaxLabel];

	ImGui::BeginPropertyTable("Collection");
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));

	std::memcpy(label, mLabel.c_str(), mLabel.size() + 1);

	ImGui::Property("Label", [&]()
		{
			ImGui::InputText("##Label", label, MaxLabel, ImGuiInputTextFlags_CharsNoBlank);
		});

	ImGui::PopStyleColor();
	ImGui::EndPropertyTable();

	mLabel = label;
}
