#pragma once

#include "../../Scenes/Scene.hpp"

namespace LRTR {

	class TinyGLTFScene : public Scene {
	public:
		explicit TinyGLTFScene(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::string& name,
			const size_t maxFrameCount);

		~TinyGLTFScene() = default;
	};

}