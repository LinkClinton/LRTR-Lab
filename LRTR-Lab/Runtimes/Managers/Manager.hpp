#pragma once

#include "../../Core/Noncopyable.hpp"
#include "../RuntimeSharing.hpp"

#include <memory>

namespace LRTR {

	class Manager : public Noncopyable {
	public:
		explicit Manager(
			const std::shared_ptr<RuntimeSharing>& sharing)
			: mRuntimeSharing(sharing) {}

		~Manager() = default;

		virtual void update(float delta) {};
	protected:
		std::shared_ptr<RuntimeSharing> mRuntimeSharing;
	};

}