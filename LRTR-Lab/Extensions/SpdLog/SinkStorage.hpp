#pragma once

#pragma warning(disable : 4275)

#include "../../Shared/Color.hpp"

#include <CodeRed/Shared/DebugReport.hpp>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

#include <vector>

namespace LRTR {

	struct LogMessage {
		std::string Message;

		ColorF Color;
	};

	template<typename Mutex>
	class SinkStorage : public spdlog::sinks::base_sink<Mutex>, public CodeRed::DebugListener {
	public:
		SinkStorage();

		~SinkStorage() = default;

		auto begin() const;

		auto end() const;
		
		auto messages() const noexcept -> const std::vector<LogMessage>&;
	protected:
		void receive(const std::string& message) override;
		
		void sink_it_(const spdlog::details::log_msg& msg) override;

		void flush_() override;
	private:
		const ColorF Red = ColorF(1, 0, 0, 1);
		const ColorF Cyan = ColorF(0, 1, 1, 1);
		const ColorF Green = ColorF(0, 1, 0, 1);
		const ColorF White = ColorF(1, 1, 1, 1);
		const ColorF Yellow = ColorF(1, 1, 0, 1);
		const ColorF CodeRed = ColorF(1, 0, 0, 1);
		
		std::vector<LogMessage> mMessages;

		std::unordered_map<spdlog::level::level_enum, ColorF> mColors;
	};

	using SinkStorageMultiThread = SinkStorage<std::mutex>;
	using SinkStorageSingleThread = SinkStorage<spdlog::details::null_mutex>;

	template <typename Mutex>
	SinkStorage<Mutex>::SinkStorage()
	{
		mColors[spdlog::level::trace] = White;
		mColors[spdlog::level::debug] = Cyan;
		mColors[spdlog::level::info] = Green;
		mColors[spdlog::level::warn] = Yellow;
		mColors[spdlog::level::err] = Red;
		mColors[spdlog::level::critical] = Red;
		mColors[spdlog::level::off] = White;
	}

	template <typename Mutex>
	auto SinkStorage<Mutex>::begin() const
	{
		return mMessages.begin();
	}

	template <typename Mutex>
	auto SinkStorage<Mutex>::end() const
	{
		return mMessages.end();
	}

	template <typename Mutex>
	auto SinkStorage<Mutex>::messages() const noexcept -> const std::vector<LogMessage>& 
	{
		return mMessages;
	}

	template <typename Mutex>
	void SinkStorage<Mutex>::receive(const std::string& message)
	{
		mMessages.push_back({ message, CodeRed });
	}

	template <typename Mutex>
	void SinkStorage<Mutex>::sink_it_(const spdlog::details::log_msg& msg) {
		fmt::memory_buffer formatted;

		spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

		mMessages.push_back({ fmt::to_string(formatted), mColors[msg.level] });
	}

	template <typename Mutex>
	void SinkStorage<Mutex>::flush_() {}

}
