#pragma once
#include <string>

namespace rss {

struct LoadResult {
    std::string body;           // Содержимое (XML)
    std::string etag;           // Заголовок ETag
    std::string lastModified;   // Заголовок Last-Modified
    bool notModified = false;   // true, если сервер ответил 304
};

class FeedLoader {
public:
    virtual ~FeedLoader() = default;
    // Загружает URL. Принимает прошлые etag/lastModified для кэша.
    virtual LoadResult load(const std::string& url, 
                            const std::string& etag = "", 
                            const std::string& lastModified = "") = 0;
};

} // namespace rss