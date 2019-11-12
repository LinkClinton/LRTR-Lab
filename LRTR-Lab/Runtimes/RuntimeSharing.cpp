#include "RuntimeSharing.hpp"
#include "LabApp.hpp"

LRTR::RuntimeSharing::RuntimeSharing(LabApp* labApp) :
	mLabApp(labApp)
{
}

auto LRTR::RuntimeSharing::uiManager() const noexcept -> std::shared_ptr<UIManager>
{
	return mLabApp->mUIManager;
}

auto LRTR::RuntimeSharing::sceneManager() const noexcept -> std::shared_ptr<SceneManager>
{
	return mLabApp->mSceneManager;
}

auto LRTR::RuntimeSharing::device() const noexcept -> std::shared_ptr<CodeRed::GpuLogicalDevice>
{
	return mLabApp->mDevice;
}
