#pragma once

#include "../System.hpp"

namespace LRTR {

	class MotionCameraUpdateSystem : public UpdateSystem {
	public:
		explicit MotionCameraUpdateSystem(const std::shared_ptr<RuntimeSharing>& sharing);

		~MotionCameraUpdateSystem() = default;

		void update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta) override;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	private:		
		Vector2f mLastMousePosition = Vector2f(0);

		bool mFirstUpdate = true;
	};
	
}