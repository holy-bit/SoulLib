#include "debug/Debug.h"
#include <vector>
#include <mutex>
#include <iostream>

namespace Debug {

namespace {
    std::vector<std::shared_ptr<LogChannel>> logChannels;
    DebugConfig currentConfig;
    std::mutex logMutex;
    
    // Función interna para imprimir mensajes a la consola
    void PrintToConsole(const std::string& colorCode, const std::string& msg) {
        std::cout << colorCode << msg << "\033[0m" << std::endl;
    }
}

void Manager::Init(const DebugConfig& cfg) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentConfig = cfg;
    logChannels.clear();
}

void Manager::Shutdown() {
    std::lock_guard<std::mutex> lock(logMutex);
    logChannels.clear();
}

void Manager::RegisterChannel(std::shared_ptr<LogChannel> channel) {
    std::lock_guard<std::mutex> lock(logMutex);
    logChannels.push_back(channel);
}

void Manager::Log(LogLevel lvl, const std::string& msg, const SourceInfo& info) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (lvl < currentConfig.minLogLevel) {
        return;
    }

    // Apply ANSI color codes for terminal output
    std::string colorCode;
    switch (lvl) {
        case LogLevel::Warning:
            colorCode = "\033[33m"; // Yellow
            break;
        case LogLevel::Error:
            colorCode = "\033[31m"; // Red
            break;
        default:
            colorCode = "\033[0m"; // Reset
            break;
    }

    // Print the message with color using our internal function
    PrintToConsole(colorCode, msg);

    LogEntry entry{lvl, msg, info};
    for (const auto& channel : logChannels) {
        if (channel->GetLevel() <= lvl) {
            channel->Write(entry);
        }
    }
}

void Manager::SetConfig(const DebugConfig& cfg) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentConfig = cfg;
}

// Implementación de las funciones de la API pública
namespace {
    Manager& GetManager() {
        static Manager manager;
        return manager;
    }
}

void Init(const DebugConfig& cfg) {
    GetManager().Init(cfg);
}

void Shutdown() {
    GetManager().Shutdown();
}

void Debug(const std::string& msg, const SourceInfo& info) {
    GetManager().Log(LogLevel::Debug, msg, info);
}

void Warning(const std::string& msg, const SourceInfo& info) {
    GetManager().Log(LogLevel::Warning, msg, info);
}

void Error(const std::string& msg, const SourceInfo& info) {
    GetManager().Log(LogLevel::Error, msg, info);
}

} // namespace Debug