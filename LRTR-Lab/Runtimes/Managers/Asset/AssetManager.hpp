#pragma once

#include "../../../Shared/Accelerators/Group.hpp"
#include "../Manager.hpp"

#include "Components/AssetComponent.hpp"

#include <string>

namespace LRTR {

	class AssetManager : public Manager {
	public:
		explicit AssetManager(const std::shared_ptr<RuntimeSharing>& sharing);

		~AssetManager() = default;
		
		void update(float delta) override;

		void addComponent(
			const std::string& name, 
			const std::shared_ptr<AssetComponent>& component);

		void removeComponent(const std::string& name);

		auto components() const noexcept -> const StringGroup<std::shared_ptr<AssetComponent>>&;
	private:
		StringGroup<std::shared_ptr<AssetComponent>> mComponents;
	};
	
}