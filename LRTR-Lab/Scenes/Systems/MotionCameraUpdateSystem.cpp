#include "MotionCameraUpdateSystem.hpp"

#include "../../Runtimes/Managers/Input/InputManager.hpp"
#include "../../Runtimes/Managers/UI/UIManager.hpp"

#include "../../Shared/Rectangle.hpp"

#include "../Cameras/Components/MotionProperty.hpp"
#include "../Cameras/MotionCamera.hpp"

LRTR::MotionCameraUpdateSystem::MotionCameraUpdateSystem(const std::shared_ptr<RuntimeSharing>& sharing) :
	UpdateSystem(sharing)
{
	
}

void LRTR::MotionCameraUpdateSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	const auto inputManager = mRuntimeSharing->inputManager();
	const auto uiManager = mRuntimeSharing->uiManager();
	
	const auto mousePosition = inputManager->mousePosition();
	const auto scenePosition = uiManager->components().at("View.Scene")->position();
	const auto sceneSize = uiManager->components().at("View.Scene")->size();
	const auto sceneRegion = RectangleF(
		scenePosition.x, scenePosition.y, 
		scenePosition.x + sceneSize.x, scenePosition.y + sceneSize.y
	);

	// we only rotate the camera when the mouse is in region of scene view
	if (sceneRegion.contain(mLastMousePosition) && sceneRegion.contain(mousePosition)) {

		const auto offset = mousePosition - mLastMousePosition;

		for (const auto& shape : shapes) {
			if (shape.second->hasComponent<Projective>() &&
				shape.second->hasComponent<MotionProperty>() &&
				shape.second->hasComponent<TransformWrap>()) {
				const auto transform = shape.second->component<TransformWrap>();
				const auto perspective = std::static_pointer_cast<Perspective>(shape.second->component<Projective>());
				const auto motionProperty = shape.second->component<MotionProperty>();

				const auto realOffset = motionProperty->sensitivity() * offset;

				auto yaw = glm::yaw(transform->rotation());
				auto pitch = glm::pitch(transform->rotation()) - realOffset.y;
				auto roll = glm::roll(transform->rotation()) - realOffset.x;

				if (pitch > glm::radians(179.f)) pitch = glm::radians(179.f);
				if (pitch < glm::radians(1.f)) pitch = glm::radians(1.f);

				transform->set(transform->translation(),
					QuaternionF(Vector3f(pitch, yaw, roll)),
					transform->scale());
			}
		}
		
	}
	
	mLastMousePosition = mousePosition;
}

auto LRTR::MotionCameraUpdateSystem::typeName() const noexcept -> std::string
{
	return "MotionCameraUpdateSystem";
}

auto LRTR::MotionCameraUpdateSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(MotionCameraUpdateSystem);
}
