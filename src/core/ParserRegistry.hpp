#pragma once
#include <vector>
#include <memory>
#include "FeedParser.hpp"

namespace rss {

class ParserRegistry {
public:
    void add(FeedParserPtr parser) {
        parsers_.push_back(std::move(parser));
    }

    // Перебирает парсеры, находит первый подходящий (canParse) и запускает (parse)
    Feed parse(const std::string& xml) const {
        for (const auto& parser : parsers_) {
            if (parser->canParse(xml)) {
                return parser->parse(xml);
            }
        }
        return Feed{}; // Ничего не подошло
    }

private:
    std::vector<FeedParserPtr> parsers_;
};

} // namespace rss