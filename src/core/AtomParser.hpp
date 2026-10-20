#pragma once
#include "FeedParser.hpp"

namespace rss {
class AtomParser : public FeedParser {
public:
    bool canParse(const std::string& xml) const override;
    Feed parse(const std::string& xml) const override;
};
} // namespace rss