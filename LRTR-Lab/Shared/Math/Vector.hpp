#pragma once

#include <glm/glm.hpp>

namespace LRTR {

	template<typename T>
	using Vector1 = glm::vec<1, T, glm::packed_highp>;

	template<typename T>
	using Vector2 = glm::vec<2, T, glm::packed_highp>;

	template<typename T>
	using Vector3 = glm::vec<3, T, glm::packed_highp>;

	template<typename T>
	using Vector4 = glm::vec<4, T, glm::packed_highp>;
}