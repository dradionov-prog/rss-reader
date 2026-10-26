#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>
#include <algorithm>

#include "AppController.hpp"
#include "JsonStateStore.hpp"
#include "HttpLoader.hpp"
#include "ParserRegistry.hpp"
#include "Rss2Parser.hpp"
#include "AtomParser.hpp"

int main() {
    std::string home = std::getenv("HOME") ? std::getenv("HOME") : "";
    std::string statePath = home + "/.config/rss-reader/state.json";
    
    auto stateStore = std::make_shared<rss::JsonStateStore>(statePath);
    auto registry = std::make_shared<rss::ParserRegistry>();
    registry->add(std::make_shared<rss::Rss2Parser>());
    registry->add(std::make_shared<rss::AtomParser>());
    auto loader = std::make_shared<rss::HttpLoader>();

    auto initialState = stateStore->load();
    if (initialState.subscriptions.empty()) {
        initialState.subscriptions.push_back("https://habr.com/ru/rss/best/");
        stateStore->save(initialState);
    }

    rss::AppController app(stateStore, registry, loader);
    app.loadKeybindings("config/keybindings.conf");

    auto screen = ftxui::ScreenInteractive::Fullscreen();

    std::string input_url = "";
    auto input_url_component = ftxui::Input(&input_url, "Вставьте ссылку на RSS");

    std::vector<std::string> feed_names;
    auto update_feeds = [&]() {
        feed_names.clear();
        for (const auto& f : app.getFeeds()) feed_names.push_back(f.title.empty() ? f.url : f.title);
    };
    update_feeds();
    
    int selected_feed = 0;
    ftxui::MenuOption feed_option;
    feed_option.on_enter = [&]() {
        app.getSession().currentFeedUrl = app.getFeeds()[selected_feed].url;
        app.getSession().currentScreen = rss::Screen::ArticleList;
        app.refreshInBackground(); 
        screen.PostEvent(ftxui::Event::Custom);
    };
    auto feed_menu = ftxui::Menu(&feed_names, &selected_feed, feed_option);

    std::vector<std::string> article_titles;
    int selected_article = 0;
    
    auto update_article_list = [&]() {
        article_titles.clear();
        for (const auto& f : app.getFeeds()) {
            if (f.url == app.getSession().currentFeedUrl) {
                for (const auto& a : f.articles) {
                    bool isRead = app.getState().readFlags.count(a.id) > 0;
                    article_titles.push_back((isRead ? "[x] " : "[ ] ") + a.title);
                }
            }
        }
        if (article_titles.empty()) article_titles.push_back("(Нет статей или загрузка...)");
    };
    update_article_list();

    ftxui::MenuOption article_option;
    article_option.on_enter = [&]() {
        for (const auto& f : app.getFeeds()) {
            if (f.url == app.getSession().currentFeedUrl) {
                if (static_cast<std::size_t>(selected_article) < f.articles.size()) {
                    std::string id = f.articles[selected_article].id;
                    app.getSession().currentArticleId = id;
                    app.getSession().currentScreen = rss::Screen::ArticleView;
                    app.markRead(id);
                }
                break;
            }
        }
    };
    auto article_menu = ftxui::Menu(&article_titles, &selected_article, article_option);

    int scroll_y = 0;
    bool is_typing_url = false;

    auto renderer = ftxui::Renderer(feed_menu, [&]() {
        using namespace ftxui;

        auto header = hbox({ text(" RSS Reader v1.0 ") | bold | bgcolor(Color::Blue), text(" (Press ? for help) ") | dim }) | border;
        std::string statusText = app.getSession().statusMessage;
        if (app.isLoading()) statusText = " [~] Loading... " + statusText;
        auto status = text(statusText) | bgcolor(Color::GrayDark);

        Element main_content;
        auto currentScreen = app.getSession().currentScreen;

        if (is_typing_url) {
            main_content = vbox({
                text("Добавление новой RSS-ленты") | bold | center,
                separator(),
                input_url_component->Render(),
                text(""),
                text("Нажмите Enter для сохранения или Esc для отмены") | dim
            }) | border | center;
        } 
        else if (currentScreen == rss::Screen::Help) {
            main_content = vbox({
                text("Справка:") | bold, separator(),
                text(" a     — добавить новую ленту"),
                text(" d     — удалить выбранную ленту"), // <-- Добавили подсказку
                text(" o     — открыть оригинал в браузере"),
                text(" Enter — открыть / прочитать"),
                text(" Стрелки — навигация / скролл"),
                text(" Esc   — назад"),
                text(" r     — обновить ленту"),
                text(" q     — выход"),
            }) | border | center;

        } else if (currentScreen == rss::Screen::ArticleList) {
            if (app.isLoading()) update_article_list();
            main_content = vbox({
                text("Лента: " + app.getSession().currentFeedUrl) | bold | underlined,
                article_menu->Render() | frame | flex
            }) | border | flex;

        } else if (currentScreen == rss::Screen::ArticleView) {
            std::string text_content = app.fetchFullArticle(app.getSession().currentArticleId);
            
            std::vector<std::string> lines;
            std::stringstream ss(text_content);
            std::string line;
            while (std::getline(ss, line)) lines.push_back(line);
            
            Elements visible_lines;
            int end = std::min(static_cast<int>(lines.size()), scroll_y + 20);
            for (int i = scroll_y; i < end; ++i) {
                visible_lines.push_back(text(lines[i]));
            }
            
            if (end >= static_cast<int>(lines.size())) {
                visible_lines.push_back(text("--- Конец статьи ---") | dim | center);
                visible_lines.push_back(text("Нажмите 'o' чтобы открыть оригинал в браузере") | color(Color::Green) | center);
            }

            main_content = vbox({
                text("Чтение статьи (Стрелки — скролл, 'o' — браузер, Esc — назад)") | bold | underlined,
                vbox(std::move(visible_lines)) | frame | flex
            }) | border | flex;

        } else {
            main_content = feed_menu->Render() | frame | flex;
        }

        return vbox({ header, main_content | flex, status });
    });

    renderer |= ftxui::CatchEvent([&](ftxui::Event event) {
        if (is_typing_url) {
            if (event == ftxui::Event::Escape) {
                is_typing_url = false;
                input_url.clear();
                return true;
            }
            if (event == ftxui::Event::Return) {
                app.addSubscription(input_url);
                update_feeds();
                is_typing_url = false;
                input_url.clear();
                return true;
            }
            return input_url_component->OnEvent(event);
        }

        auto currentScreen = app.getSession().currentScreen;

        if (event == ftxui::Event::Character('a')) {
            is_typing_url = true;
            return true;
        }

        // УДАЛЕНИЕ ЛЕНТЫ
        if (event == ftxui::Event::Character('d') && currentScreen == rss::Screen::FeedList) {
            if (selected_feed >= 0 && static_cast<std::size_t>(selected_feed) < app.getFeeds().size()) {
                std::string url_to_delete = app.getFeeds()[selected_feed].url;
                app.removeSubscription(url_to_delete);
                update_feeds();
                if (selected_feed >= static_cast<int>(feed_names.size()) && selected_feed > 0) {
                    selected_feed--;
                }
            }
            return true;
        }

        if (event == ftxui::Event::Character(app.getKey("quit")[0])) {
            screen.ExitLoopClosure()();
            return true;
        }
        if (event == ftxui::Event::Character(app.getKey("help")[0])) {
            app.getSession().currentScreen = rss::Screen::Help;
            return true;
        }
        if (event == ftxui::Event::Escape) {
            if (currentScreen == rss::Screen::Help) {
                app.getSession().currentScreen = rss::Screen::FeedList;
            } else if (currentScreen == rss::Screen::ArticleView) {
                app.getSession().currentScreen = rss::Screen::ArticleList;
            } else if (currentScreen == rss::Screen::ArticleList) {
                app.getSession().currentScreen = rss::Screen::FeedList;
            }
            return true;
        }

        if (currentScreen == rss::Screen::ArticleView && event == ftxui::Event::Character('o')) {
            for (const auto& f : app.getFeeds()) {
                if (f.url == app.getSession().currentFeedUrl) {
                    for (const auto& a : f.articles) {
                        if (a.id == app.getSession().currentArticleId) {
                            std::string cmd = "xdg-open \"" + a.link + "\" > /dev/null 2>&1 &";
                            std::system(cmd.c_str());
                            app.getSession().statusMessage = "Открываю в браузере...";
                            return true;
                        }
                    }
                }
            }
        }

        if (event == ftxui::Event::Return) {
            if (currentScreen == rss::Screen::ArticleList) {
                for (const auto& f : app.getFeeds()) {
                    if (f.url == app.getSession().currentFeedUrl) {
                        if (static_cast<std::size_t>(selected_article) < f.articles.size()) {
                            std::string id = f.articles[selected_article].id;
                            app.getSession().currentArticleId = id;
                            app.getSession().currentScreen = rss::Screen::ArticleView;
                            scroll_y = 0;
                            app.markRead(id);
                        }
                        break;
                    }
                }
                return true;
            } 
            else if (currentScreen == rss::Screen::FeedList) {
                app.getSession().currentFeedUrl = app.getFeeds()[selected_feed].url;
                app.getSession().currentScreen = rss::Screen::ArticleList;
                app.refreshInBackground(); 
                screen.PostEvent(ftxui::Event::Custom);
                return true;
            }
        }

        if (currentScreen == rss::Screen::ArticleList) {
            if (event == ftxui::Event::ArrowUp) {
                if (selected_article > 0) selected_article--;
                return true;
            }
            if (event == ftxui::Event::ArrowDown) {
                if (static_cast<std::size_t>(selected_article) < article_titles.size() - 1) selected_article++;
                return true;
            }
        } else if (currentScreen == rss::Screen::FeedList) {
            if (event == ftxui::Event::ArrowUp) {
                if (selected_feed > 0) selected_feed--;
                return true;
            }
            if (event == ftxui::Event::ArrowDown) {
                if (static_cast<std::size_t>(selected_feed) < feed_names.size() - 1) selected_feed++;
                return true;
            }
        } else if (currentScreen == rss::Screen::ArticleView) {
            if (event == ftxui::Event::ArrowDown || event == ftxui::Event::Character('j')) {
                scroll_y++;
                return true;
            }
            if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('k')) {
                if (scroll_y > 0) scroll_y--;
                return true;
            }
        }

        if (event == ftxui::Event::Character(app.getKey("reload")[0])) {
            if (currentScreen == rss::Screen::ArticleList) {
                app.refreshInBackground();
                screen.PostEvent(ftxui::Event::Custom);
            }
            return true;
        }

        return false;
    });

    std::atomic<bool> keep_running{true};
    std::thread refresher([&]() {
        while (keep_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if (!app.isLoading()) {
                screen.PostEvent(ftxui::Event::Custom);
            }
        }
    });

    screen.Loop(renderer);
    keep_running = false;
    if (refresher.joinable()) refresher.join();

    return 0;
}