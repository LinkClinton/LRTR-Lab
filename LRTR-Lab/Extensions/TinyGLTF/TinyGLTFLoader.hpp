#pragma once

#include "TinyGLTFScene.hpp"

namespace LRTR {

	class TinyGLTFLoader {
	public:
		static auto loadScene(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::string& sceneName,
			const std::string& fileName,
			const Transform& transform = Transform())
			-> std::shared_ptr<TinyGLTFScene>;
	};
	
}