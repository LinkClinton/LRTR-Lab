#include "Perspective.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::Perspective::Perspective() :
	Perspective(MathUtility::pi<float>() * 0.25f, 1920.0f, 1080.0f, 0.1f, 1000.0f)
{
	
}

LRTR::Perspective::Perspective(
	const float fovy,
	const float width,
	const float height,
	const float zNear,
	const float zFar) :
	Projective(Transform::perspectiveFov(fovy, width, height, zNear, zFar)),
	mFovy(fovy), mWidth(width), mHeight(height), mZNear(zNear), mZFar(zFar)
{
	
}

auto LRTR::Perspective::typeName() const noexcept -> std::string
{
	return "Perspective";
}

auto LRTR::Perspective::typeIndex() const noexcept -> std::type_index
{
	return typeid(Perspective);
}

void LRTR::Perspective::onProperty()
{
	auto degrees = glm::degrees(mFovy);

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable("Fovy");
	ImGui::Property("Fovy", [&]() { ImGui::InputFloat("##Fovy", &degrees, 0, 0, 1); });
	ImGui::BeginPropertyTable("Width");
	ImGui::Property("Width", [&] {ImGui::InputFloat("##Width", &mWidth); });
	ImGui::BeginPropertyTable("Height");
	ImGui::Property("Height", [&] {ImGui::InputFloat("##Height", &mHeight); });
	ImGui::BeginPropertyTable("ZNear");
	ImGui::Property("ZNear", [&] {ImGui::InputFloat("##ZNear", &mZNear); });
	ImGui::BeginPropertyTable("ZFar");
	ImGui::Property("ZFar", [&] {ImGui::InputFloat("##ZFar", &mZFar); });

	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();
	
	mFovy = glm::radians(degrees);

	mCameraToScreen = Transform::perspectiveFov(mFovy, mWidth, mHeight, mZNear, mZFar);
}
