#include "AssimpScene.hpp"

LRTR::AssimpScene::AssimpScene(
	const std::shared_ptr<RuntimeSharing>& sharing, 
	const std::string& name,
	const size_t maxFrameCount) : Scene(name, sharing->device(), maxFrameCount)
{
	
}
