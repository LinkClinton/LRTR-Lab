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

auto LRTR::RuntimeSharing::device() const noexcept -> std::shared_ptr<CodeRed::GpuLogicalDevice>
{
	return mLabApp->mDevice;
}
