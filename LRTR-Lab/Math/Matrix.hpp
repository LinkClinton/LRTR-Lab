#pragma once

#include <glm/glm.hpp>

namespace LRTR {

	template<typename T>
	using Matrix4x4 = glm::mat<4, 4, T, glm::packed_highp>;

}