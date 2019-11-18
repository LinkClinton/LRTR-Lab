#pragma once

namespace LRTR {

	class Renderable {
	public:
		Renderable() = default;

		virtual ~Renderable() = default;

		virtual void show() { mVisibility = true; }

		virtual void hide() { mVisibility = false; }
	protected:
		bool mVisibility = true;
	};
	
}