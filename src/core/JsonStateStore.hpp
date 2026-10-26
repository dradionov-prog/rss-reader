#pragma once
#include "StateStore.hpp"

namespace rss {
class JsonStateStore : public StateStore {
public:
    explicit JsonStateStore(std::string path);
    AppState load() override;
    void save(const AppState& state) override;
private:
    std::string path_;
};
} // namespace rss