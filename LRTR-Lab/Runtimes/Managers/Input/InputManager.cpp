#include "InputManager.hpp"

#include <Windows.h>

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

auto LRTR::InputManager::mousePosition() const noexcept -> Vector2f
{
	return mRuntimeSharing->mousePosition();
}

auto LRTR::InputManager::keyState(const KeyCode& keyCode) const noexcept -> bool
{
	return KEYDOWN(static_cast<int>(keyCode));
}
