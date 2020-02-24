#pragma once

#include "../../Component.hpp"

namespace LRTR {

	class MotionProperty : public Component {
	public:
		MotionProperty() : MotionProperty(0.05f, 1.0f) {}
		
		explicit MotionProperty(const float sensitivity, const float speed);

		~MotionProperty() = default;

		auto sensitivity() const noexcept -> float { return mSensitivity; }

		auto speed() const noexcept -> float { return mSpeed; }
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		float mSensitivity = 0.05f;
		float mSpeed = 1.0f;
	};
	
}
