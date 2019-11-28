#include "System.hpp"

LRTR::System::System(const std::shared_ptr<RuntimeSharing>& sharing) :
	mRuntimeSharing(sharing)
{
	
}

auto LRTR::System::typeName() const noexcept -> std::string
{
	return "System";
}

auto LRTR::System::typeIndex() const noexcept -> std::type_index
{
	return typeid(System);
}

LRTR::UpdateSystem::UpdateSystem(const std::shared_ptr<RuntimeSharing>& sharing) :
	System(sharing)
{
}

auto LRTR::UpdateSystem::typeName() const noexcept -> std::string
{
	return "UpdateSystem";
}

auto LRTR::UpdateSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(UpdateSystem);
}

LRTR::RenderSystem::RenderSystem(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device, 
	size_t maxFrameCount)
	: UpdateSystem(sharing), mDevice(device), mFrameResources(maxFrameCount)
{
	
}

auto LRTR::RenderSystem::typeName() const noexcept -> std::string
{
	return "RenderSystem";
}

auto LRTR::RenderSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(RenderSystem);
}
