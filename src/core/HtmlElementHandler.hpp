#pragma once
#include <string>
#include "pugixml.hpp"
#include <sstream>
namespace rss {

struct RenderContext {
    std::string result;
    int width;
    std::string currentLine;

    void appendWord(const std::string& word) {
        // Исправляем ошибку сравнения типов, которая была раньше, 
        // используя static_cast для width
        if (currentLine.length() + word.length() + 1 > static_cast<std::size_t>(width)) {
            result += currentLine + "\n";
            currentLine = word;
        } else {
            if (!currentLine.empty()) currentLine += " ";
            currentLine += word;
        }
    }

    void flushLine() {
        if (!currentLine.empty()) {
            result += currentLine + "\n";
            currentLine.clear();
        }
    }

    void newline() { flushLine(); }

    void append(const std::string& text) {
        // 1. Заменяем HTML-сущности на нормальные символы
        std::string cleanText = text;
        std::string nbsp = "&nbsp;";
        std::size_t pos = 0;
        while ((pos = cleanText.find(nbsp, pos)) != std::string::npos) {
            cleanText.replace(pos, nbsp.length(), " ");
            pos += 1;
        }

        // 2. Разбиваем на слова и добавляем
        std::stringstream ss(cleanText);
        std::string word;
        while (ss >> word) appendWord(word);
    }
};

class HtmlElementHandler {
public:
    virtual ~HtmlElementHandler() = default;
    virtual const char* tagName() const = 0;
    virtual void handle(const pugi::xml_node& node, RenderContext& ctx) = 0;
};

} // namespace rss