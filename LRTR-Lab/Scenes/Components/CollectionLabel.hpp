#pragma once

#include "../Component.hpp"

namespace LRTR {

	class CollectionLabel : public Component {
	public:
		CollectionLabel() = default;

		explicit CollectionLabel(const std::string& label);

		~CollectionLabel() = default;

		void set(const std::string& label);
		
		auto label() const noexcept -> std::string;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		std::string mLabel = "Collection";
	};
	
}