#pragma once

#include "../../Shared/Accelerators/Group.hpp"
#include "../Component.hpp"

namespace LRTR {

	class CameraGroup : public Component {
	public:
		CameraGroup() = default;

		~CameraGroup() = default;

		void addCamera(const std::string& name);

		void removeCamera(const std::string& name);

		auto cameras() const noexcept -> const StringGroup<std::string>&;

		auto current() const noexcept -> std::string;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		StringGroup<std::string> mCameras;

		std::string mCurrent;
	};
	
}