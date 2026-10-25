#pragma once
#include "FeedLoader.hpp"

namespace rss {
class HttpLoader : public FeedLoader {
public:
    LoadResult load(const std::string& url, 
                    const std::string& etag = "", 
                    const std::string& lastModified = "") override;
};
} // namespace rss