#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "ParserRegistry.hpp"
#include "Rss2Parser.hpp"
#include "AtomParser.hpp"

using namespace rss;

int main() {
    std::cout << "rss-reader v0.2.0 (parsers ready)\n";

    // Регистрируем парсеры
    ParserRegistry registry;
    registry.add(std::make_shared<Rss2Parser>());
    registry.add(std::make_shared<AtomParser>());

    // Читаем файл (для теста можно взять наш тестовый файл)
    std::ifstream file("../testdata/rss/test_rss.xml");
    if (!file) {
        std::cerr << "File not found\n";
        return 1;
    }
    std::stringstream ss;
    ss << file.rdbuf();

    // Парсим
    Feed feed = registry.parse(ss.str());

    // Выводим результат
    std::cout << "Feed: " << feed.title << "\n";
    std::cout << "Articles: " << feed.articles.size() << "\n\n";
    for (const auto& art : feed.articles) {
        std::cout << "- " << art.title << " (" << art.link << ")\n";
        std::cout << "  HTML: " << art.contentHtml.substr(0, 30) << "...\n";
    }

    return 0;
}