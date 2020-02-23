#pragma once

namespace LRTR {

	class Shadowable {
	public:
		Shadowable() = default;

		virtual ~Shadowable() = default;

		void cast() { mShadowCast = true; }

		void unCast() { mShadowCast = false; }
		
		auto isCast() const noexcept -> bool { return mShadowCast; }
	protected:
		bool mShadowCast = true;
	};

}