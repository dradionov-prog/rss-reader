#include "HtmlHandlers.hpp"
#include <iostream>

namespace rss {

void ParagraphHandler::handle(const pugi::xml_node& node, RenderContext& ctx) {
    ctx.newline();
    std::string text = node.text().get();
    // Перенос по словам мы сделаем позже, пока просто пишем текст
    ctx.append(text);
    ctx.newline();
    ctx.newline(); // Пустая строка после абзаца
}

void HeadingHandler::handle(const pugi::xml_node& node, RenderContext& ctx) {
    ctx.newline();
    ctx.append("=== " + std::string(node.text().get()) + " ===");
    ctx.newline();
    ctx.newline();
}

void LinkHandler::handle(const pugi::xml_node& node, RenderContext& ctx) {
    // В ТЗ сказано: ссылки сносками [1] со списком адресов в конце.
    // Пока для простоты выводим текст ссылки и адрес в скобках.
    std::string text = node.text().get();
    std::string href = node.attribute("href").value();
    ctx.append(text + " [" + href + "]");
}

} // namespace rss