#include "CameraGroup.hpp"

#include "../../Extensions/ImGui/ImGui.hpp"

void LRTR::CameraGroup::addCamera(const std::string& name)
{	
	mCameras.insert({ name, name });
}

void LRTR::CameraGroup::removeCamera(const std::string& name)
{
	mCameras.erase(name);
}

auto LRTR::CameraGroup::cameras() const noexcept -> const StringGroup<std::string>& 
{
	return mCameras;
}

auto LRTR::CameraGroup::current() const noexcept -> std::string
{
	return mCurrent;
}

auto LRTR::CameraGroup::typeName() const noexcept -> std::string
{
	return "CameraGroup";
}

auto LRTR::CameraGroup::typeIndex() const noexcept -> std::type_index
{
	return typeid(CameraGroup);
}

void LRTR::CameraGroup::onProperty()
{
	ImGui::BeginPropertyTable("Combo");

	//if we do not set current camera
	//we will use the first camera
	if (mCurrent.empty() && mCameras.size() != 0)
		mCurrent = mCameras.begin()->first;
	
	ImGui::Property("Camera", [&]()
		{
			if (ImGui::BeginCombo("##Camera", mCurrent.c_str())) {
				for (const auto& camera : mCameras) {
					const auto selected = (mCurrent == camera.first);

					if (ImGui::Selectable(camera.first.c_str(), selected))
						mCurrent = camera.first;
					
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		});

	ImGui::EndPropertyTable();
}
