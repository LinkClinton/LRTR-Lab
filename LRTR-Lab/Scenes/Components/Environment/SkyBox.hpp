#pragma once

#include <CodeRed/Interface/GpuResource/GpuTexture.hpp>

#include "../../../Core/Renderable.hpp"
#include "../../Component.hpp"

namespace LRTR {

	class SkyBox : public Component, public Renderable {
	public:
		explicit SkyBox(const std::shared_ptr<CodeRed::GpuTexture>& cubeMap);

		auto cubeMap() const noexcept -> std::shared_ptr<CodeRed::GpuTexture>;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		std::shared_ptr<CodeRed::GpuTexture> mCubeMap;
	};
	
}