#include "LinesGrid.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

#include "CoordinateSystem.hpp"

LRTR::LinesGrid::LinesGrid(
	const RectangleF& area,
	const size_t xCount, 
	const size_t yCount, 
	const ColorF& color) :
	LinesGrid(area, xCount, yCount,
		Vector3f(1, 0, 0),
		Vector3f(0, 1, 0),
		color)
{
}

LRTR::LinesGrid::LinesGrid(
	const RectangleF& area,
	const size_t xCount,
	const size_t yCount,
	const Vector3f& xAxis,
	const Vector3f& yAxis,
	const ColorF& color) :
	LinesGrid(area, xCount, yCount, xAxis, yAxis,
		Vector3f(0), color)
{
}

LRTR::LinesGrid::LinesGrid(
	const RectangleF& area,
	const size_t xCount,
	const size_t yCount,
	const Vector3f& xAxis,
	const Vector3f& yAxis,
	const Vector3f& origin,
	const ColorF& color) :
	mOrigin(origin), mXAxis(xAxis), mYAxis(yAxis),
	mArea(area), mXCount(xCount), mYCount(yCount),
	mColor(color)
{
	updateLines();
}

auto LRTR::LinesGrid::origin() const noexcept -> Vector3f
{
	return mOrigin;
}

auto LRTR::LinesGrid::xAxis() const noexcept -> Vector3f
{
	return mXAxis;
}

auto LRTR::LinesGrid::yAxis() const noexcept -> Vector3f
{
	return mYAxis;
}

auto LRTR::LinesGrid::area() const noexcept -> RectangleF
{
	return mArea;
}

auto LRTR::LinesGrid::xCount() const noexcept -> size_t
{
	return mXCount;
}

auto LRTR::LinesGrid::yCount() const noexcept -> size_t
{
	return mYCount;
}

auto LRTR::LinesGrid::typeName() const noexcept -> std::string
{
	return "LinesGrid";
}

auto LRTR::LinesGrid::typeIndex() const noexcept -> std::type_index
{
	return typeid(LinesGrid);
}

void LRTR::LinesGrid::onProperty()
{
	static auto currentAxis = static_cast<size_t>(Axis::eX);

	static const auto EditFlags =
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_Float;
	
	static const char* AxesName[] = {
		"X",
		"Y",
		"Z"
	};

	auto isChanged = false;
	
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable("Origin");
	ImGui::Property("Origin   X", [&]() { isChanged ^= ImGui::InputFloat("##X0", &mOrigin.x); });
	ImGui::Property("         Y", [&]() { isChanged ^= ImGui::InputFloat("##Y0", &mOrigin.y); });
	ImGui::Property("         Z", [&]() { isChanged ^= ImGui::InputFloat("##Z0", &mOrigin.z); });

	ImGui::PopStyleColor();
	
	ImGui::AxisProperty(2, currentAxis);

	auto& axis = currentAxis == 0 ? mXAxis : mYAxis;
	
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable(AxesName[currentAxis]);
	ImGui::Property("Location X", [&]() { isChanged ^= ImGui::InputFloat("##X1", &axis.x); });
	ImGui::Property("         Y", [&]() { isChanged ^= ImGui::InputFloat("##Y1", &axis.y); });
	ImGui::Property("         Z", [&]() { isChanged ^= ImGui::InputFloat("##Z1", &axis.z); });

	ImGui::BeginPropertyTable("Color");
	ImGui::Property("Color    RGBA", [&]()
		{
			isChanged ^= ImGui::ColorEdit4("##ColorEdit4", reinterpret_cast<float*>(&mColor), EditFlags);
		});
	
	ImGui::BeginPropertyTable("Area");
	ImGui::Property("Area     MinX", [&]() { isChanged ^= ImGui::InputFloat("##MinX", &mArea.Min.x); });
	ImGui::Property("         MinY", [&]() { isChanged ^= ImGui::InputFloat("##MinY", &mArea.Min.y); });
	ImGui::Property("         MaxX", [&]() { isChanged ^= ImGui::InputFloat("##MaxX", &mArea.Max.x); });
	ImGui::Property("         MaxY", [&]() { isChanged ^= ImGui::InputFloat("##MaxY", &mArea.Max.y); });

	auto x = static_cast<int>(mXCount);
	auto y = static_cast<int>(mYCount);
	
	ImGui::BeginPropertyTable("Subdivision");
	ImGui::Property("Subdivision X", [&]() { isChanged ^= ImGui::InputInt("##TestX", &x); });
	ImGui::Property("            Y", [&]() { isChanged ^= ImGui::InputInt("##TestY", &y); });

	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() {ImGui::Checkbox("##Visibility", &mVisibility); });
	
	ImGui::PopStyleColor();

	ImGui::EndPropertyTable();

	x = MathUtility::max(x, 1);
	y = MathUtility::max(y, 1);

	//the min of area can not greater than max
	mArea.Min = MathUtility::min(mArea.Min, mArea.Max);
	mXCount = static_cast<size_t>(x);
	mYCount = static_cast<size_t>(y);

	if (isChanged) updateLines();
}

void LRTR::LinesGrid::updateLines()
{
	mLines.clear();

	const auto xAxis = MathUtility::normalize(mXAxis);
	const auto yAxis = MathUtility::normalize(mYAxis);

	const auto width = mArea.Max.x - mArea.Min.x;
	const auto height = mArea.Max.y - mArea.Min.y;
	const auto xDelta = width / mXCount;
	const auto yDelta = height / mYCount;
	
	if (width == 0 || height == 0) return;
	
	for (size_t index = 0; index <= mXCount; index++) {
		const auto offset = mArea.Min.x + index * xDelta;
		const auto begin = mOrigin + xAxis * offset + yAxis * mArea.Min.y;
		const auto end = begin + yAxis * height;

		mLines.push_back({ begin, end, mColor });
	}
	
	for (size_t index = 0; index <= mYCount; index++) {
		const auto offset = mArea.Min.y + index * yDelta;
		const auto begin = mOrigin + yAxis * offset + xAxis * mArea.Min.x;
		const auto end = begin + xAxis * width;

		mLines.push_back({ begin, end, mColor });
	}
}
