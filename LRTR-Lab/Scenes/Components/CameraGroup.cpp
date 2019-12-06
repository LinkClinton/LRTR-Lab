#include "CameraGroup.hpp"

#include "../Components/CollectionLabel.hpp"
#include "../../Extensions/ImGui/ImGui.hpp"

#include "../Shape.hpp"


void LRTR::CameraGroup::addCamera(const std::shared_ptr<Shape>& shape)
{
	if (mCameras.empty()) mCurrent = shape->identity();
	
	mCameras.insert({ shape->identity(), shape });
}

void LRTR::CameraGroup::removeCamera(const Identity& identity)
{
	mCameras.erase(identity);

	if (mCameras.empty()) mCurrent = std::numeric_limits<Identity>::max();
}

auto LRTR::CameraGroup::cameras() const noexcept -> const Group<Identity, std::shared_ptr<Shape>>& 
{
	return mCameras;
}

auto LRTR::CameraGroup::current() const noexcept -> std::shared_ptr<Shape>
{
	return mCurrent == std::numeric_limits<Identity>::max() ? nullptr : mCameras.at(mCurrent);
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

	ImGui::Property("Camera", [&]()
		{
			const auto currentName = mCurrent == std::numeric_limits<Identity>::max() ? "Empty" :
				(current()->hasComponent<CollectionLabel>() ?
					current()->component<CollectionLabel>()->name() :
					"Unknown");
		
			if (ImGui::BeginCombo("##Camera", currentName.c_str())) {
				for (const auto& camera : mCameras) {
					const auto selected = (mCurrent == camera.first);
					const auto name = camera.second->hasComponent<CollectionLabel>() ?
						camera.second->component<CollectionLabel>()->name() :
						"Unknown";
					
					if (ImGui::Selectable(name.c_str(), selected))
						mCurrent = camera.first;
					
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		});

	ImGui::EndPropertyTable();
}
