#pragma once

#include "../../../Core/Renderable.hpp"
#include "../../../Core/Blurable.hpp"
#include "../../Component.hpp"

namespace LRTR {

	class Material : public Component, public Renderable, public Blurable {
	public:
		Material() = default;

		~Material() = default;
	};
	
}