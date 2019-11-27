#pragma once

#include "AssimpScene.hpp"

namespace LRTR {

	class AssimpLoader {
	public:
		static auto loadScene(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::string& sceneName,
			const std::string& fileName)
			-> std::shared_ptr<AssimpScene>;
	};
	
}