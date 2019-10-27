#pragma once

#pragma warning(disable : 4275)

#include <spdlog\spdlog.h>

#define __ENABLE__CODE__LOCATION__

#ifdef __ENABLE__CODE__LOCATION__
#define LRTR_WARNING(...) SPDLOG_WARN(__VA_ARGS__);
#define LRTR_ERROR(...) SPDLOG_ERROR(__VA_ARGS__);
#define LRTR_INFO(...) SPDLOG_INFO(__VA_ARGS__);
#else
#define LRTR_WARNING(...) spdlog::warn(__VA_ARGS__);
#define LRTR_ERROR(...) spdlog::error(__VA_ARGS__);
#define LRTR_INFO(...) spdlog::info(__VA_ARGS__);
#endif

#define LRTR_WARNING_IF(condition, ...)  if (condition) LRTR_WARNING(__VA_ARGS__);
#define LRTR_ERROR_IF(condition, ...) if (condition) LRTR_ERROR(__VA_ARGS__);
#define LRTR_INFO_IF(condition, ...) if (condition) LRTR_INFO(__VA_ARGS__);

#ifdef _DEBUG
#define LRTR_DEBUG_WARNING(...) LRTR_WARNING(__VA_ARGS__);
#define LRTR_DEBUG_ERROR(...) LRTR_ERROR(__VA_ARGS__);
#define LRTR_DEBUG_INFO(...) LRTR_INFO(__VA_ARGS__);

#define LRTR_DEBUG_WARNING_IF(condition, ...) if (condition) LRTR_DEBUG_WARNING(__VA_ARGS__);
#define LRTR_DEBUG_ERROR_IF(condition, ...) if (condition) LRTR_DEBUG_ERROR(__VA_ARGS__);
#define LRTR_DEBUG_INFO_IF(condition, ...) if (condition) LRTR_DEBUG_INFO(__VA_ARGS__);
#else
#define LRTR_DEBUG_WARNING(...)
#define LRTR_DEBUG_ERROR(...)
#define LRTR_DEBUG_INFO(...)

#define LRTR_DEBUG_WARNING_IF(condition, ...)
#define LRTR_DEBUG_ERROR_IF(condition, ...)
#define LRTR_DEBUG_INFO_IF(condition, ...)
#endif // _DEBUG

