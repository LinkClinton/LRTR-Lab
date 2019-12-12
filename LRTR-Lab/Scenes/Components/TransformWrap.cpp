#include "TransformWrap.hpp"

#include "../../Extensions/ImGui/ImGui.hpp"

LRTR::TransformWrap::TransformWrap(const Vector3f& translation, const Vector4f& rotation, const Vector3f& scale)
{
	set(translation, rotation, scale);
}

LRTR::TransformWrap::TransformWrap(const Vector3f& translation, const QuaternionF& rotation, const Vector3f& scale)
{
	set(translation, rotation, scale);
}

void LRTR::TransformWrap::set(const Vector3f& translation, const Vector4f& rotation, const Vector3f& scale)
{
	mTranslation = translation;
	mRotation = Vector3f(rotation) == Vector3f(0) ? QuaternionF() : glm::angleAxis(rotation.w, Vector3f(rotation));
	mScale = scale;

	mTransform = Transform(mTranslation, mRotation, mScale);
}

void LRTR::TransformWrap::set(const Vector3f& translation, const QuaternionF& rotation, const Vector3f& scale)
{
	mTranslation = translation;
	mRotation = rotation;
	mScale = scale;

	mTransform = Transform(mTranslation, mRotation, mScale);
}

auto LRTR::TransformWrap::translation() const noexcept -> Vector3f
{
	return mTranslation;
}

auto LRTR::TransformWrap::rotation() const noexcept -> QuaternionF
{
	return mRotation;
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
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable("Translate");
	ImGui::Property("Location X", [&]() { ImGui::InputFloat("##X0", &mTranslation.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y0", &mTranslation.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z0", &mTranslation.z); });

	ImGui::BeginPropertyTable("Rotate");
	ImGui::Property("Rotate   X", [&]() { ImGui::InputFloat("##X1", &mRotation.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y1", &mRotation.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z1", &mRotation.z); });
	ImGui::Property("         W", [&]() { ImGui::InputFloat("##W1", &mRotation.w); });

	ImGui::BeginPropertyTable("Scale");
	ImGui::Property("Scale    X", [&]() { ImGui::InputFloat("##X2", &mScale.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y2", &mScale.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z2", &mScale.z); });
	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();

	mTransform = Transform(mTranslation, mRotation, mScale);
}
