#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

#include <SoulLibExport.h>

namespace SoulLib {
namespace Utils {

class SOULLIB_API JsonUtils {
public:
    using json = nlohmann::json;

    static std::optional<json> LoadJson(const std::string& path);
    static bool SaveJson(const std::string& path, const json& data);
    
    template<typename T>
    static T GetValue(const json& j, const std::string& key, const T& defaultValue) {
        if (j.contains(key) && !j[key].is_null()) {
            try {
                return j[key].get<T>();
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
};

} // namespace Utils
} // namespace SoulLib
