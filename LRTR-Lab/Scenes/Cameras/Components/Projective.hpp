#pragma once

#include "../../../Shared/Transform.hpp"
#include "../../Component.hpp"

namespace LRTR {

	class Projective : public Component {
	public:
		Projective() = default;
		
		explicit Projective(const Transform& cameraToScreen) :
			mCameraToScreen(cameraToScreen) {}

		~Projective() = default;

		auto toScreen() const noexcept -> Transform { return mCameraToScreen; }
		
		auto typeName() const noexcept -> std::string override {
			return "Projective";
		}

		auto typeIndex() const noexcept -> std::type_index override {
			return typeid(Projective);
		}
	protected:
		Transform mCameraToScreen;
	};

}