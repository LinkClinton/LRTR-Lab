#include "LinesMesh.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::Line::Line(const Vector3f& begin, const Vector3f& end, const ColorF& color) :
	Begin(begin), End(end), Color(color)
{
}

LRTR::LinesMesh::LinesMesh(const std::vector<Line>& lines) :
	mLines(lines)
{
}


auto LRTR::LinesMesh::line(size_t index) const -> Line
{
	return mLines[index];
}

auto LRTR::LinesMesh::liens() const noexcept -> const std::vector<Line>& 
{
	return mLines;
}

auto LRTR::LinesMesh::size() const -> size_t
{
	return mLines.size();
}

auto LRTR::LinesMesh::typeName() const noexcept -> std::string
{
	return "LinesMesh";
}

auto LRTR::LinesMesh::typeIndex() const noexcept -> std::type_index
{
	return typeid(LinesMesh);
}

void LRTR::LinesMesh::onProperty()
{
	static const auto LineName = [](size_t index) {return std::to_string(index); };
	
	const static auto EditFlags =
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_Float;

	mCurrentLine = MathUtility::clamp(mCurrentLine, static_cast<size_t>(0), mLines.size() - 1);

	auto currentName = mLines.empty() ? "Empty" : LineName(mCurrentLine);
	
	ImGui::BeginPropertyTable("Combo");
	ImGui::Property("Line", [&]()
		{
			if (ImGui::BeginCombo("##Line", currentName.c_str())) {
				for (size_t index = 0; index < mLines.size(); index++) {
					const auto selected = (mCurrentLine == index);

					if (ImGui::Selectable(LineName(index).c_str(), selected))
						mCurrentLine = index;
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		});

	auto emptyLine = Line();
	auto& line = mLines.empty() ? emptyLine : mLines[mCurrentLine];
	
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable(LineName(mCurrentLine).c_str());
	ImGui::Property("Begin    X", [&]() { ImGui::InputFloat("##X0", &line.Begin.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y0", &line.Begin.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z0", &line.Begin.z); });
	ImGui::BeginPropertyTable(LineName(mCurrentLine).c_str());
	ImGui::Property("End      X", [&]() { ImGui::InputFloat("##X1", &line.End.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y1", &line.End.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z1", &line.End.z); });
	ImGui::BeginPropertyTable(LineName(mCurrentLine).c_str());
	ImGui::Property("Color    RGBA", [&]()
		{
			ImGui::ColorEdit4("##ColorEdit4", reinterpret_cast<float*>(&line.Color), EditFlags);
		});
	
	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() {ImGui::Checkbox("##Visibility", &IsRendered); });

	ImGui::PopStyleColor();

	ImGui::EndPropertyTable();
}
