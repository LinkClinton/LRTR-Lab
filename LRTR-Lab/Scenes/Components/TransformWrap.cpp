#include "TransformWrap.hpp"

#include "../../Extensions/ImGui/ImGui.hpp"

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
	auto degrees = glm::degrees(mRotate.w);

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable("Translate");
	ImGui::Property("Location X", [&]() { ImGui::InputFloat("##X0", &mTranslate.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y0", &mTranslate.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z0", &mTranslate.z); });

	ImGui::BeginPropertyTable("Rotate");
	ImGui::Property("Rotate   W", [&]() { ImGui::InputFloat("##W1", &degrees, 0, 0, 1); });
	ImGui::Property("         X", [&]() { ImGui::InputFloat("##X1", &mRotate.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y1", &mRotate.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z1", &mRotate.z); });

	ImGui::BeginPropertyTable("Scale");
	ImGui::Property("Scale    X", [&]() { ImGui::InputFloat("##X2", &mScale.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y2", &mScale.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z2", &mScale.z); });
	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();

	mRotate.w = glm::radians(degrees);
	
	mTransform = Transform(mTranslate, mRotate, mScale);
}
