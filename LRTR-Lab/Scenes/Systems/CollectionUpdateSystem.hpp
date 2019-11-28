#pragma once

#include "../../Shared/Accelerators/Group.hpp"

#include "../System.hpp"

namespace LRTR {
	
	class CollectionUpdateSystem : public UpdateSystem {
	public:
		using Collection = std::vector<std::pair<std::string, std::shared_ptr<Shape>>>;
		
		explicit CollectionUpdateSystem(
			const std::shared_ptr<RuntimeSharing>& sharing);

		~CollectionUpdateSystem() = default;

		void update(
			const StringGroup<std::shared_ptr<Shape>>& shapes, 
			float delta) override;

		auto collections() const noexcept -> const StringGroup<Collection>&;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	private:
		
		StringGroup<Collection> mCollections;
	};
	
}