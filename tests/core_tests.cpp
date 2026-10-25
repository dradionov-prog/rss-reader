#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include "Rss2Parser.hpp"
#include "AtomParser.hpp"
#include "ParserRegistry.hpp"
#include <iostream>
#include "HtmlRenderer.hpp"
using namespace rss;

std::string readFile(const std::string& path) {
    // Ищем файл в нескольких местах: из папки build, из корня проекта
    std::ifstream f("../" + path); 
    if (!f) f.open(path); 
    if (!f) f.open("../../" + path); 
    
    if (!f) {
        std::cerr << "FAILED TO OPEN FILE: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

TEST(RssParserTest, CanParseRss) {
    Rss2Parser parser;
    std::string xml = readFile("testdata/rss/test_rss.xml");
    EXPECT_TRUE(parser.canParse(xml));
}

TEST(RssParserTest, ParsesCorrectly) {
    Rss2Parser parser;
    std::string xml = readFile("testdata/rss/test_rss.xml");
    Feed feed = parser.parse(xml);
    EXPECT_EQ(feed.title, "Test RSS Feed");
    ASSERT_EQ(feed.articles.size(), 1);
    EXPECT_EQ(feed.articles[0].id, "guid-1");
}

TEST(AtomParserTest, ParsesCorrectly) {
    AtomParser parser;
    std::string xml = readFile("testdata/rss/test_atom.xml");
    Feed feed = parser.parse(xml);
    EXPECT_EQ(feed.title, "Test Atom Feed");
    ASSERT_EQ(feed.articles.size(), 1);
}

TEST(RegistryTest, ChoosesCorrectParser) {
    ParserRegistry reg;
    reg.add(std::make_shared<Rss2Parser>());
    reg.add(std::make_shared<AtomParser>());
    std::string rssXml = readFile("testdata/rss/test_rss.xml");
    Feed f1 = reg.parse(rssXml);
    EXPECT_EQ(f1.title, "Test RSS Feed");
}
TEST(HtmlRendererTest, RendersSimpleHtml) {
    HtmlRenderer renderer;
    std::string html = "<h1>Title</h1><p>Hello <a href=\"http://example.com\">world</a></p>";
    std::string text = renderer.render(html, 80);

    // Проверяем, что заголовок отрисовался
    EXPECT_NE(text.find("=== Title ==="), std::string::npos);
    // Проверяем, что текст и ссылка на месте
    EXPECT_NE(text.find("Hello"), std::string::npos);
    EXPECT_NE(text.find("world [http://example.com]"), std::string::npos);
}

TEST(HtmlRendererTest, SkipsUnknownTagsButKeepsText) {
    HtmlRenderer renderer;
    std::string html = "<custom>Hidden text</custom><p>Visible</p>";
    std::string text = renderer.render(html, 80);
    
    // Неизвестный тег <custom> должен быть пропущен, но текст "Hidden text" остаться
    EXPECT_NE(text.find("Hidden text"), std::string::npos);
}