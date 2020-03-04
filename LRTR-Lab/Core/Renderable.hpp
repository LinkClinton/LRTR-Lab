#pragma once

namespace LRTR {

	class Renderable {
	public:
		Renderable() = default;

		virtual ~Renderable() = default;

		bool IsRendered;
	};
	
}