#pragma once

#include "../../../Shared/Math/Math.hpp"

#include "../Manager.hpp"
#include "KeyCode.hpp"

namespace LRTR {

	class InputManager : public Manager {
	public:
		explicit InputManager(const std::shared_ptr<RuntimeSharing>& sharing) : Manager(sharing) {}

		~InputManager() = default;

		auto mousePosition() const noexcept -> Vector2f;

		auto keyState(const KeyCode& keyCode) const noexcept -> bool;
	};
	
}
