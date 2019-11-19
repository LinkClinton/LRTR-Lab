#include "System.hpp"

LRTR::System::System(const std::shared_ptr<RuntimeSharing>& sharing) :
	mRuntimeSharing(sharing)
{
	
}

LRTR::UpdateSystem::UpdateSystem(const std::shared_ptr<RuntimeSharing>& sharing) :
	System(sharing)
{
}

LRTR::RenderSystem::RenderSystem(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device, 
	size_t maxFrameCount)
	: UpdateSystem(sharing), mDevice(device), mFrameResources(maxFrameCount)
{
	
}
