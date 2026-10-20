#pragma once
#include <string>
#include <chrono>

namespace rss {

enum class LoadStatus {
    Unknown,
    Loading,
    Ok,
    Error
};

struct Article {
    std::string id;
    std::string title;
    std::string link;
    std::chrono::system_clock::time_point date;
    std::string contentHtml;
    bool isRead = false;

    Article() = default;

    Article(std::string id, std::string title, std::string link, std::string html = "")
        : id(std::move(id)), title(std::move(title)), link(std::move(link)), contentHtml(std::move(html)) {}
};

} // namespace rss