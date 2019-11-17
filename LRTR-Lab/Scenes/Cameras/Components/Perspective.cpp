#include "Perspective.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

LRTR::Perspective::Perspective() :
	Perspective(MathUtility::pi<float>() * 0.25f, 1920.0f, 1080.0f, 0.0f, 1000.0f)
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
	const static auto genColumn = [](const char* text, float* data, int precision = 3)
	{
		static std::string head = "##";
		
		ImGui::Separator();
		ImGui::AlignTextToFramePadding();

		ImGui::Text(text); ImGui::NextColumn();
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
		ImGui::InputFloat((head + text).c_str(), data, 0, 0 , precision); ImGui::NextColumn();
		ImGui::PopStyleColor();
	};

	auto degrees = glm::degrees(mFovy);
	
	ImGui::Columns(2);

	genColumn("Fovy", &degrees, 1);
	genColumn("Width", &mWidth);
	genColumn("Height", &mHeight);
	genColumn("ZNear", &mZNear);
	genColumn("ZFar", &mZFar);

	ImGui::Columns(1);
	ImGui::Separator();
	
	/*ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12);
	ImGui::InputFloat("Fovy", &mFovy);
	ImGui::InputFloat("Width", &mWidth);
	ImGui::InputFloat("Height", &mHeight);
	ImGui::InputFloat("ZNear", &mZNear);
	ImGui::InputFloat("ZFar", &mZFar);
	ImGui::PopStyleVar();*/

	mFovy = glm::radians(degrees);

	mCameraToScreen = Transform::perspectiveFov(mFovy, mWidth, mHeight, mZNear, mZFar);
}
