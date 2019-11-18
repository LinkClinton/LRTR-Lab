#include "TransformWrap.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

LRTR::TransformWrap::TransformWrap(const Vector3f& translate, const Vector4f& rotate, const Vector3f& scale)
{
	set(translate, rotate, scale);
}

void LRTR::TransformWrap::set(const Vector3f& translate, const Vector4f& rotate, const Vector3f& scale)
{
	mTranslate = translate;
	mRotate = rotate;
	mScale = scale;

	mTransform = Transform(mTranslate, mRotate, mScale);
}

auto LRTR::TransformWrap::translate() const noexcept -> Vector3f
{
	return mTranslate;
}

auto LRTR::TransformWrap::rotate() const noexcept -> Vector4f
{
	return mRotate;
}

auto LRTR::TransformWrap::scale() const noexcept -> Vector3f
{
	return mScale;
}

auto LRTR::TransformWrap::transform() const noexcept -> Transform
{
	return mTransform;
}

auto LRTR::TransformWrap::typeName() const noexcept -> std::string
{
	return "Transform";
}

auto LRTR::TransformWrap::typeIndex() const noexcept -> std::type_index
{
	return typeid(TransformWrap);
}

void LRTR::TransformWrap::onProperty()
{
	const static auto genColumn = [](const char* text, const char* id, float* data, const char* format = "%.3f")
	{
		static std::string head = "##";

		ImGui::AlignTextToFramePadding();

		ImGui::Text(text); ImGui::NextColumn();
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
		ImGui::InputFloat((head + id + text).c_str(), data, 0, 0, format); ImGui::NextColumn();
		ImGui::PopStyleColor();
	};


	auto degrees = glm::degrees(mRotate.w);

	ImGui::Columns(2, "Translate");
	ImGui::Separator();
	
	genColumn("Location X", "0", &mTranslate.x);
	genColumn("         Y", "0", &mTranslate.y);
	genColumn("         Z", "0", &mTranslate.z);
	
	ImGui::Columns(2, "Rotate");
	ImGui::Separator();
	
	genColumn("Rotate   W", "1", &degrees, "%.1f");
	genColumn("         X", "1", &mRotate.x);
	genColumn("         Y", "1", &mRotate.y);
	genColumn("         Z", "1", &mRotate.z);

	ImGui::Columns(2, "Scale");
	ImGui::Separator();
	
	genColumn("Scale    X", "2", &mScale.x);
	genColumn("         Y", "2", &mScale.y);
	genColumn("         Z", "2", &mScale.z);

	ImGui::Columns(1);
	ImGui::Separator();

	mRotate.w = glm::radians(degrees);
	
	mTransform = Transform(mTranslate, mRotate, mScale);
}
