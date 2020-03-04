#pragma once

namespace LRTR {

	class Shadowable {
	public:
		Shadowable() = default;

		virtual ~Shadowable() = default;

		bool IsShadowed = true;
	};

}