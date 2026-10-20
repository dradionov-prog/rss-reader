#pragma once
#include <string>
#include <memory>
#include "Feed.hpp"

namespace rss {

class FeedParser {
public:
    virtual ~FeedParser() = default;

    // Проверяет, может ли этот парсер обработать данный XML
    virtual bool canParse(const std::string& xml) const = 0;

    // Разбирает XML и возвращает заполненный Feed
    virtual Feed parse(const std::string& xml) const = 0;
};

using FeedParserPtr = std::shared_ptr<FeedParser>;

} // namespace rss