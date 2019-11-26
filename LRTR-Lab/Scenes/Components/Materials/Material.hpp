#pragma once

#include "../../../Core/Renderable.hpp"
#include "../../Component.hpp"

namespace LRTR {

	class Material : public Component, public Renderable {
	public:
		Material() = default;

		~Material() = default;
	};
	
}