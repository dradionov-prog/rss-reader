#include "HtmlRenderer.hpp"
#include "HtmlHandlers.hpp"
#include "pugixml.hpp"

namespace rss {

HtmlRenderer::HtmlRenderer() {
    // Регистрация обработчиков (Фабричный метод / Стратегия)
    handlers_["p"] = std::make_unique<ParagraphHandler>();
    handlers_["h1"] = std::make_unique<HeadingHandler>();
    handlers_["h2"] = std::make_unique<HeadingHandler>();
    handlers_["a"] = std::make_unique<LinkHandler>();
}

void HtmlRenderer::walk(const pugi::xml_node& node, RenderContext& ctx) const {
    if (node.type() == pugi::node_pcdata) {
        // Это просто текст внутри тега. Добавляем в буфер.
        // В реальности тут нужен умный перенос по словам (wrap).
        ctx.append(node.value());
        return;
    }

    auto it = handlers_.find(node.name());
    if (it != handlers_.end()) {
        it->second->handle(node, ctx);
    } else {
        // Неизвестный тег — пропускаем, но идем внутрь (требование ТЗ)
        for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
            walk(child, ctx);
        }
    }
}

std::string HtmlRenderer::render(const std::string& html, int width) const {
    pugi::xml_document doc;
    // pugixml может парсить HTML, но требует, чтобы он был XML-подобным (закрытые теги).
    // В реальном проекте тут стоит использовать lexbor или gumbo.
    doc.load_string(html.c_str());

    RenderContext ctx;
    ctx.width = width;
    walk(doc, ctx);
    
    return ctx.result;
}

} // namespace rss