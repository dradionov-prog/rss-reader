#pragma once
#include "HtmlElementHandler.hpp"

namespace rss {

class ParagraphHandler : public HtmlElementHandler {
public:
    const char* tagName() const override { return "p"; }
    void handle(const pugi::xml_node& node, RenderContext& ctx) override;
};

class HeadingHandler : public HtmlElementHandler {
public:
    const char* tagName() const override { return "h1"; } // Для простоты пока только h1
    void handle(const pugi::xml_node& node, RenderContext& ctx) override;
};

class LinkHandler : public HtmlElementHandler {
public:
    const char* tagName() const override { return "a"; }
    void handle(const pugi::xml_node& node, RenderContext& ctx) override;
};

} // namespace rss