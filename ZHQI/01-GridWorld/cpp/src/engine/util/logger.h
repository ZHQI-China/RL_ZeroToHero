#pragma once

#include <spdlog/spdlog.h>

#define LogTrace(...)	spdlog::trace(__VA_ARGS__)
#define LogInfo(...)	spdlog::info(__VA_ARGS__)
#define LogDebug(...)	spdlog::debug(__VA_ARGS__)
#define LogWarn(...)	spdlog::warn(__VA_ARGS__)
#define LogError(...)	spdlog::error(__VA_ARGS__)

// todo: 写入日志文件，一些宏



// Core log macros
#define ENGINE_TRACE	LogTrace
#define ENGINE_INFO		LogInfo
#define ENGINE_WARN		LogWarn
#define ENGINE_ERROR	LogError
#define ENGINE_CRITICAL LogError

// Client log macros
#define GAME_TRACE       LogTrace
#define GAME_INFO          LogInfo
#define GAME_WARN          LogWarn
#define GAME_ERROR         LogError
#define GAME_CRITICAL      LogError
