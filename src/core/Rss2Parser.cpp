#include "Rss2Parser.hpp"
#include "pugixml.hpp"
#include <iostream>

namespace rss {

bool Rss2Parser::canParse(const std::string& xml) const {
    // Ищем тег <rss> в начале документа
    pugi::xml_document doc;
    if (!doc.load_string(xml.c_str())) return false;
    return doc.child("rss") != nullptr;
}

Feed Rss2Parser::parse(const std::string& xml) const {
    pugi::xml_document doc;
    if (!doc.load_string(xml.c_str())) return Feed{};

    pugi::xml_node rss = doc.child("rss");
    pugi::xml_node channel = rss.child("channel");

    Feed feed;
    feed.title = channel.child("title").text().get();
    feed.status = LoadStatus::Ok;

    for (pugi::xml_node item = channel.child("item"); item; item = item.next_sibling("item")) {
        Article art;
        art.title = item.child("title").text().get();
        art.link = item.child("link").text().get();
        
        // GUID обязателен в RSS, но если нет — используем ссылку
        art.id = item.child("guid").text().get();
        if (art.id.empty()) art.id = art.link;

        art.contentHtml = item.child("description").text().get();
        feed.articles.push_back(std::move(art));
    }

    return feed;
}

} // namespace rss