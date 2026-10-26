#include "JsonStateStore.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace rss {

JsonStateStore::JsonStateStore(std::string path) : path_(std::move(path)) {}

AppState JsonStateStore::load() {
    AppState state;
    std::ifstream f(path_);
    if (!f) return state; // Файла нет — возвращаем пустое состояние

    try {
        json j;
        f >> j;
        if (j.contains("subscriptions")) 
            state.subscriptions = j["subscriptions"].get<std::vector<std::string>>();
        if (j.contains("readFlags")) 
            state.readFlags = j["readFlags"].get<std::map<std::string, bool>>();
    } catch (...) {
        std::cerr << "Failed to parse state JSON\n";
    }
    return state;
}

void JsonStateStore::save(const AppState& state) {
    json j;
    j["subscriptions"] = state.subscriptions;
    j["readFlags"] = state.readFlags;

    // Создаем папки, если их нет
    fs::path p(path_);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());

    // Атомарная запись: сначала во временный файл, потом rename
    std::string tmp = path_ + ".tmp";
    {
        std::ofstream f(tmp);
        f << j.dump(2);
    }
    fs::rename(tmp, path_);
}

} // namespace rss