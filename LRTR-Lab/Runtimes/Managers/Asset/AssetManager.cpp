#include "AssetManager.hpp"

#include "Components/MeshDataAssetComponent.hpp"

LRTR::AssetManager::AssetManager(const std::shared_ptr<RuntimeSharing>& sharing) :
	Manager(sharing)
{
	addComponent("MeshData", std::make_shared<MeshDataAssetComponent>(sharing, sharing->device()));
}

void LRTR::AssetManager::update(float delta)
{
	
}

void LRTR::AssetManager::addComponent(const std::string& name, const std::shared_ptr<AssetComponent>& component)
{
	mComponents.insert({ name, component });
}

void LRTR::AssetManager::removeComponent(const std::string& name)
{
	mComponents.erase(name);
}

auto LRTR::AssetManager::components() const noexcept -> const StringGroup<std::shared_ptr<AssetComponent>>& 
{
	return mComponents;
}
