#include "AtomParser.hpp"
#include "pugixml.hpp"

namespace rss {

bool AtomParser::canParse(const std::string& xml) const {
    pugi::xml_document doc;
    if (!doc.load_string(xml.c_str())) return false;
    // Atom использует <feed> с пространством имен
    return doc.child("feed") != nullptr;
}

Feed AtomParser::parse(const std::string& xml) const {
    pugi::xml_document doc;
    if (!doc.load_string(xml.c_str())) return Feed{};

    pugi::xml_node feedNode = doc.child("feed");

    Feed feed;
    feed.title = feedNode.child("title").text().get();
    feed.status = LoadStatus::Ok;

    for (pugi::xml_node entry = feedNode.child("entry"); entry; entry = entry.next_sibling("entry")) {
        Article art;
        art.title = entry.child("title").text().get();
        art.id = entry.child("id").text().get();
        
        // В Atom ссылка лежит в атрибуте href
        pugi::xml_node link = entry.child("link");
        if (link) art.link = link.attribute("href").value();

        art.contentHtml = entry.child("content").text().get();
        if (art.contentHtml.empty()) 
            art.contentHtml = entry.child("summary").text().get();

        feed.articles.push_back(std::move(art));
    }

    return feed;
}

} // namespace rss