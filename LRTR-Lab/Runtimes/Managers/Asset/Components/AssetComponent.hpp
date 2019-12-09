#pragma once

#include "../../../../Core/Noncopyable.hpp"
#include "../../../RuntimeSharing.hpp"

#include <memory>

namespace LRTR {

	class AssetComponent : public Noncopyable {
	public:
		explicit AssetComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
			mRuntimeSharing(sharing) {}

		~AssetComponent() = default;
	protected:
		std::shared_ptr<RuntimeSharing> mRuntimeSharing;
	};
	
}