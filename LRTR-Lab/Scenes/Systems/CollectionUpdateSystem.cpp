#include "CollectionUpdateSystem.hpp"

#include "../Components/CollectionLabel.hpp"

LRTR::CollectionUpdateSystem::CollectionUpdateSystem(const std::shared_ptr<RuntimeSharing>& sharing) :
	UpdateSystem(sharing)
{
	
}

void LRTR::CollectionUpdateSystem::update(const StringGroup<std::shared_ptr<Shape>>& shapes, float delta)
{
	mCollections.clear();

	for (const auto& shape : shapes) {
		//if the shape does not have the component, we will think it has a label called "Collection"
		const auto label = shape.second->hasComponent<CollectionLabel>() ? 
			shape.second->component<CollectionLabel>()->label() : "Collection";

		mCollections[label].push_back(shape);
	}
}

auto LRTR::CollectionUpdateSystem::collections() const noexcept -> const StringGroup<Collection>& 
{
	return mCollections;
}

auto LRTR::CollectionUpdateSystem::typeName() const noexcept -> std::string
{
	return "CollectionUpdateSystem";
}

auto LRTR::CollectionUpdateSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(CollectionUpdateSystem);
}
