#pragma once

namespace LRTR {

	class Blurable {
	public:
		Blurable() = default;

		virtual ~Blurable() = default;

		bool IsBlurred = false;
	};
	
}
