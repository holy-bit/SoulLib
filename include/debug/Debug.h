#pragma once

#include <memory>
#include <string>
#include "LogLevel.h"
#include "LogChannel.h"
#include "DebugConfig.h"
#include "SourceInfo.h"

namespace Debug {

class Manager {
public:
    void Init(const DebugConfig& cfg);
    void Shutdown();
    void RegisterChannel(std::shared_ptr<LogChannel> channel);
    void Log(LogLevel lvl, const std::string& msg, const SourceInfo& info);
    void SetConfig(const DebugConfig& cfg);
};

void Init(const DebugConfig& cfg);
void Shutdown();
void Debug(const std::string& msg, const SourceInfo& info);
void Warning(const std::string& msg, const SourceInfo& info);
void Error(const std::string& msg, const SourceInfo& info);

} // namespace Debug

// Macros para simplificar el uso del sistema de depuración
// Estos macros capturan automáticamente la información de origen (archivo, línea y función)
#ifndef DEBUG_LOG
#define DEBUG_LOG(msg) Debug::Debug(msg, {__FILE__, __LINE__, __func__})
#endif

#ifndef DEBUG_WARNING
#define DEBUG_WARNING(msg) Debug::Warning(msg, {__FILE__, __LINE__, __func__})
#endif

#ifndef DEBUG_ERROR
#define DEBUG_ERROR(msg) Debug::Error(msg, {__FILE__, __LINE__, __func__})
#endif