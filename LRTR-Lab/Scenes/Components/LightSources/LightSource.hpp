#pragma once

#include "../../../Shared/Math/Math.hpp"
#include "../../../Core/Shadowable.hpp"

#include "../../Component.hpp"

namespace LRTR {

	class LightSource : public Component, public Shadowable {
	public:
		explicit LightSource(const Vector3f& intensity) :
			mIntensity(intensity) {}

		auto typeName() const noexcept -> std::string override { return "LightSource"; }

		auto typeIndex() const noexcept -> std::type_index override { return typeid(LightSource); }
	protected:
		Vector3f mIntensity;
	};
	
}