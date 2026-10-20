#pragma once
#include <string>
#include <vector>
#include "Article.hpp"

namespace rss {

struct Feed {
    std::string url;
    std::string title;
    std::vector<Article> articles;
    LoadStatus status = LoadStatus::Unknown;
    std::string errorMessage;

    Feed() = default;
    explicit Feed(std::string url) : url(std::move(url)) {}
};

} // namespace rss