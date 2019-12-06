#pragma once

#include "../Component.hpp"

namespace LRTR {

	class CollectionLabel : public Component {
	public:
		CollectionLabel() = default;

		explicit CollectionLabel(
			const std::string& label,
			const std::string& name);

		~CollectionLabel() = default;

		void set(const std::string& label, const std::string& name);
		
		auto label() const noexcept -> std::string;

		auto name() const noexcept -> std::string;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		std::string mLabel = "Collection";
		std::string mName = "Unknown";
	};
	
}