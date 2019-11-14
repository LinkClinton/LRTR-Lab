#pragma once

namespace LRTR {

	class Runtime final {
	public:
		static void initialize();
	private:
		Runtime() = default;

		~Runtime() = default;
	};
	
}