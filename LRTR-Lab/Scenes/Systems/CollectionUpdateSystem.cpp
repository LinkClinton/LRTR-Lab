#include "CollectionUpdateSystem.hpp"

#include "../Components/CollectionLabel.hpp"

LRTR::CollectionUpdateSystem::CollectionUpdateSystem(const std::shared_ptr<RuntimeSharing>& sharing) :
	UpdateSystem(sharing)
{
	
}

void LRTR::CollectionUpdateSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	mCollections.clear();

	for (const auto& shape : shapes) {
		//if the shape does not have the component, we will think it has a label called "Collection"
		const auto component = shape.second->hasComponent<CollectionLabel>() ?
			shape.second->component<CollectionLabel>() : std::make_shared<CollectionLabel>();
		
		mCollections[component->label()].push_back({ component->name(), shape.second });
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
