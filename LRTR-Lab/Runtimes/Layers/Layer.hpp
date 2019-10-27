#pragma once

#include "../../Core/Noncopyable.hpp"

namespace LRTR {

	class Layer : public Noncopyable {
	public:
		explicit Layer() = default;

		~Layer() = default;

		virtual void update(float delta) {};
	private:
	};

}