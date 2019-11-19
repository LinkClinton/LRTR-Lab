#pragma once

#include "../Shape.hpp"

namespace LRTR {

	class SceneProperty : public Shape {
	public:
		SceneProperty();

		~SceneProperty() = default;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	};
	
}