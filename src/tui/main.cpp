#include <iostream>
#include <memory>

#include "ParserRegistry.hpp"
#include "Rss2Parser.hpp"
#include "AtomParser.hpp"
#include "HttpLoader.hpp"
#include "HtmlRenderer.hpp"

using namespace rss;

int main() {
    std::cout << "rss-reader v0.3.0 (HTTP + HTML rendering)\n";

    // 1. Настраиваем реестр
    ParserRegistry registry;
    registry.add(std::make_shared<Rss2Parser>());
    registry.add(std::make_shared<AtomParser>());

    // 2. Загружаем ленту по HTTP
    HttpLoader loader;
    std::string url = "https://habr.com/ru/rss/best/"; // Возьмем реальную ленту
    std::cout << "Downloading " << url << "...\n";
    
    LoadResult res = loader.load(url);
    if (res.body.empty()) {
        std::cerr << "Failed to download or empty response.\n";
        return 1;
    }

    // 3. Парсим XML
    Feed feed = registry.parse(res.body);
    std::cout << "Feed: " << feed.title << " (" << feed.articles.size() << " articles)\n\n";

    // 4. Берем первую статью и рендерим её HTML
    if (!feed.articles.empty()) {
        const auto& art = feed.articles[0];
        std::cout << "Title: " << art.title << "\n";
        std::cout << "----------------------------------------\n";
        
        HtmlRenderer renderer;
        // Отрисовываем с шириной 60 символов
        std::string text = renderer.render(art.contentHtml, 60);
        std::cout << text << "\n";
    }

    return 0;
}