#pragma once
#include <string>
#include <vector>
#include <map>

namespace rss {

struct AppState {
    std::vector<std::string> subscriptions; // Список URL подписок
    std::map<std::string, bool> readFlags;  // articleId -> прочитано (true/false)
};

class StateStore {
public:
    virtual ~StateStore() = default;
    virtual AppState load() = 0;
    virtual void save(const AppState& state) = 0;
};

} // namespace rss