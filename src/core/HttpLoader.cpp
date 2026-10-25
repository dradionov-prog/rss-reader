#include "HttpLoader.hpp"
#include <curl/curl.h>

namespace rss {

// Коллбэк, в который libcurl пишет ответ
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* mem = static_cast<std::string*>(userp);
    mem->append(static_cast<char*>(contents), realsize);
    return realsize;
}

LoadResult HttpLoader::load(const std::string& url, const std::string& etag, const std::string& lastModified) {
    CURL* curl = curl_easy_init();
    LoadResult result;
    if (!curl) return result;

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Следовать редиректам
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);       // Таймаут 10 секунд

    // Добавляем заголовки для кэша, если они есть
    struct curl_slist* headers = nullptr;
    if (!etag.empty()) headers = curl_slist_append(headers, ("If-None-Match: " + etag).c_str());
    if (!lastModified.empty()) headers = curl_slist_append(headers, ("If-Modified-Since: " + lastModified).c_str());
    if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (res == CURLE_OK) {
        if (http_code == 304) {
            result.notModified = true; // Сервер сказал "ничего не изменилось"
        } else if (http_code == 200) {
            result.body = readBuffer;
            // В реальности тут надо читать заголовки ETag и Last-Modified (через CURLOPT_HEADERFUNCTION),
            // для упрощения на этом этапе можно оставить пустыми или сохранить код.
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}

} // namespace rss