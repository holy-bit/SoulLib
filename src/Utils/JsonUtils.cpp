#include "Utils/JsonUtils.h"
#include <fstream>

namespace SoulLib {
namespace Utils {

std::optional<JsonUtils::json> JsonUtils::LoadJson(const std::string& path) {
    try {
        std::ifstream f(path);
        if (!f.is_open()) return std::nullopt;
        
        json j;
        f >> j;
        return j;
    } catch (...) {
        return std::nullopt;
    }
}

bool JsonUtils::SaveJson(const std::string& path, const json& data) {
    try {
        std::ofstream f(path);
        if (!f.is_open()) return false;
        
        f << data.dump(4);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace Utils
} // namespace SoulLib
