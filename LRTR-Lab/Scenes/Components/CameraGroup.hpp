#pragma once

#include "../../Shared/Accelerators/Group.hpp"
#include "../Component.hpp"

namespace LRTR {

	class Shape;
	
	class CameraGroup : public Component {
	public:
		CameraGroup() = default;

		~CameraGroup() = default;

		void addCamera(const std::shared_ptr<Shape>& shape);

		void removeCamera(const Identity& identity);

		auto cameras() const noexcept -> const Group<Identity, std::shared_ptr<Shape>>&;

		auto current() const noexcept -> std::shared_ptr<Shape>;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		Group<Identity, std::shared_ptr<Shape>> mCameras;

		Identity mCurrent;
	};
	
}