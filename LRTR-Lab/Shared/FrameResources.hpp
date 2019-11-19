#pragma once

#include <unordered_map>
#include <memory>

namespace LRTR {

	class FrameResources {
	public:
		FrameResources() = default;

		explicit FrameResources(const std::vector<std::string>& names);

		explicit FrameResources(const std::initializer_list<std::string>& names);

		template<typename T>
		void set(
			const std::string& name,
			const std::shared_ptr<T>& resource);

		template<typename T>
		auto get(const std::string& name) const->std::shared_ptr<T>;
	private:
		std::unordered_map<std::string, std::shared_ptr<void>> mResources;
	};

	template <typename T>
	void FrameResources::set(const std::string& name, const std::shared_ptr<T>& resource)
	{
		mResources[name] = resource;
	}

	template <typename T>
	auto FrameResources::get(const std::string& name) const -> std::shared_ptr<T>
	{
		return std::static_pointer_cast<T>(mResources.at(name));
	}
}
