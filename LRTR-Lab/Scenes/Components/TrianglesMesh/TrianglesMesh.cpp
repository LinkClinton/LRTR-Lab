#include "TrianglesMesh.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::TrianglesMesh::TrianglesMesh(const std::vector<TriangleF>& triangles)
{
	using sort_pair = std::pair<Vector3f, size_t>;
	
	auto vertices = std::vector<sort_pair>(triangles.size() * 3);

	for (size_t index = 0; index < triangles.size(); index++) {
		const auto& triangle = triangles[index];
		
		vertices[index * 3 + 0] = { triangle.Vertices[0], index * 3 + 0 };
		vertices[index * 3 + 1] = { triangle.Vertices[1], index * 3 + 1 };
		vertices[index * 3 + 2] = { triangle.Vertices[2], index * 3 + 2 };
	}

	std::sort(vertices.begin(), vertices.end(),
		[](const sort_pair& left, const sort_pair& right)
		{
			if (left.first.x == right.first.x) {
				if (left.first.y == right.first.y)
					return left.first.z < right.first.z;

				return left.first.y < right.first.y;
			}

			return left.first.x < right.first.x;
		});

	mIndices = std::vector<unsigned>(triangles.size() * 3);

	for (size_t index = 0; index < vertices.size(); index++) {
		if (index == 0 || vertices[index].first != vertices[index - 1].first)
			mVertices.push_back(vertices[index].first);

		mIndices[vertices[index].second] = static_cast<unsigned>(mVertices.size() - 1);
	}
}

LRTR::TrianglesMesh::TrianglesMesh(const std::vector<Vector3f>& vertices, const std::vector<unsigned>& indices) :
	mVertices(vertices), mIndices(indices)
{
	
}

auto LRTR::TrianglesMesh::vertices() const noexcept -> const std::vector<Vector3f>& 
{
	return mVertices;
}

auto LRTR::TrianglesMesh::indices() const noexcept -> const std::vector<unsigned>& 
{
	return mIndices;
}

auto LRTR::TrianglesMesh::triangle(const size_t index) const -> TriangleF
{
	return TriangleF(
		mVertices[mIndices[index * 3 + 0]],
		mVertices[mIndices[index * 3 + 1]],
		mVertices[mIndices[index * 3 + 2]]
	);
}

auto LRTR::TrianglesMesh::size() const noexcept -> size_t
{
	return mIndices.size() / 3;
}

auto LRTR::TrianglesMesh::typeName() const noexcept -> std::string
{
	return "TrianglesMesh";
}

auto LRTR::TrianglesMesh::typeIndex() const noexcept -> std::type_index
{
	return typeid(TrianglesMesh);
}

void LRTR::TrianglesMesh::onProperty()
{
	static const auto TriangleName = [](size_t index) {return std::to_string(index); };
	static const auto DrawFloat = [](const char* id, float* value)
	{
		ImGui::InputFloat(id, value, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
	};
	
	const static auto EditFlags =
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_Float;

	mCurrentTriangle = MathUtility::clamp(mCurrentTriangle, static_cast<size_t>(0), mIndices.size() / 3);

	auto currentName = mIndices.empty() ? "Empty" : TriangleName(mCurrentTriangle);
	auto count = static_cast<int>(size());

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));

	ImGui::BeginPropertyTable("Triangles");
	ImGui::Property("Triangles", [&]()
		{
			ImGui::InputInt("##Count", &count, 0, 0, ImGuiInputTextFlags_ReadOnly);
		});

	ImGui::PopStyleColor();

	ImGui::BeginPropertyTable("Combo");
	ImGui::Property("Triangle", [&]()
		{
			if (ImGui::BeginCombo("##Triangle", currentName.c_str())) {
				for (size_t index = 0; index < mIndices.size() / 3; index++) {
					const auto selected = (mCurrentTriangle == index);

					if (ImGui::Selectable(TriangleName(index).c_str(), selected))
						mCurrentTriangle = index;
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		});

	auto triangle = mIndices.empty() ? TriangleF() : TrianglesMesh::triangle(mCurrentTriangle);

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable(TriangleName(mCurrentTriangle).c_str());
	ImGui::Property("V[0]     X", [&]() { DrawFloat("##X0", &triangle.Vertices[0].x); });
	ImGui::Property("         Y", [&]() { DrawFloat("##Y0", &triangle.Vertices[0].y); });
	ImGui::Property("         Z", [&]() { DrawFloat("##Z0", &triangle.Vertices[0].z); });
	ImGui::BeginPropertyTable(TriangleName(mCurrentTriangle).c_str());
	ImGui::Property("V[1]     X", [&]() { DrawFloat("##X1", &triangle.Vertices[1].x); });
	ImGui::Property("         Y", [&]() { DrawFloat("##Y1", &triangle.Vertices[1].y); });
	ImGui::Property("         Z", [&]() { DrawFloat("##Z1", &triangle.Vertices[1].z); });
	ImGui::BeginPropertyTable(TriangleName(mCurrentTriangle).c_str());
	ImGui::Property("V[2]     X", [&]() { DrawFloat("##X2", &triangle.Vertices[2].x); });
	ImGui::Property("         Y", [&]() { DrawFloat("##Y2", &triangle.Vertices[2].y); });
	ImGui::Property("         Z", [&]() { DrawFloat("##Z2", &triangle.Vertices[2].z); });

	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();
}
