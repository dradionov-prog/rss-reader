#pragma once
#include <string>
#include <map>
#include <memory>
#include "HtmlElementHandler.hpp"

namespace rss {

class HtmlRenderer {
public:
    HtmlRenderer();
    
    // Главный метод: принимает HTML и ширину окна, возвращает текст
    std::string render(const std::string& html, int width) const;

private:
    void walk(const pugi::xml_node& node, RenderContext& ctx) const;
    std::map<std::string, std::unique_ptr<HtmlElementHandler>> handlers_;
};

} // namespace rss