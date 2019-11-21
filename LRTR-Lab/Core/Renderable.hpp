#pragma once

namespace LRTR {

	class Renderable {
	public:
		Renderable() = default;

		virtual ~Renderable() = default;

		virtual void show() { mVisibility = true; }

		virtual void hide() { mVisibility = false; }

		virtual auto visibility() const -> bool { return mVisibility; }
	protected:
		bool mVisibility = true;
	};
	
}