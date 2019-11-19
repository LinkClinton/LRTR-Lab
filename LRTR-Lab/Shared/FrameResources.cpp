#include "FrameResources.hpp"

LRTR::FrameResources::FrameResources(const std::vector<std::string>& names)
{
	for (auto& name : names) mResources.insert({ name, nullptr });
}

LRTR::FrameResources::FrameResources(const std::initializer_list<std::string>& names)
{
	for (auto& name : names) mResources.insert({ name, nullptr });
}
