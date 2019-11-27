#pragma once

#include "../../Core/Noncopyable.hpp"
#include "../../Scenes/Scene.hpp"

#include <vector>

namespace LRTR {

	class AssimpScene : public Scene {
	public:
		explicit AssimpScene(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::string& name,
			const size_t maxFrameCount);

		~AssimpScene() = default;
	};
	
}