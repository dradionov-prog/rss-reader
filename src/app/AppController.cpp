#include "AppController.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace rss {

AppController::AppController(std::shared_ptr<StateStore> store,
                             std::shared_ptr<ParserRegistry> parsers,
                             std::shared_ptr<FeedLoader> loader)
    : store_(std::move(store)), parsers_(std::move(parsers)), loader_(std::move(loader)) {
    reloadData();
}

AppController::~AppController() {
    if (worker_.joinable()) {
        worker_.join();
    }
}

void AppController::reloadData() {
    std::lock_guard<std::mutex> lock(mutex_);
    appState_ = store_->load();
    feeds_.clear();
    for (const auto& url : appState_.subscriptions) {
        Feed f(url);
        f.title = url;
        feeds_.push_back(f);
    }
}

void AppController::refreshCurrentFeed() {
    if (session_.currentFeedUrl.empty()) return;
    
    LoadResult res = loader_->load(session_.currentFeedUrl);
    
    Feed newFeed;
    if (!res.body.empty()) {
        newFeed = parsers_->parse(res.body);
        newFeed.url = session_.currentFeedUrl;
        if (newFeed.articles.empty() && newFeed.title.empty()) {
            newFeed.status = LoadStatus::Error;
            newFeed.errorMessage = "Неверный формат RSS (возможно, это HTML-страница).";
        } else {
            newFeed.status = LoadStatus::Ok;
        }
    } else {
        newFeed.url = session_.currentFeedUrl;
        newFeed.status = LoadStatus::Error;
        newFeed.errorMessage = "Сеть недоступна или пустой ответ.";
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& feed : feeds_) {
        if (feed.url == session_.currentFeedUrl) {
            feed = newFeed;
            break;
        }
    }
    session_.statusMessage = "Обновлено";
}

void AppController::refreshInBackground() {
    if (loading_) return;
    loading_ = true;
    
    if (worker_.joinable()) worker_.join();

    worker_ = std::thread([this]() {
        session_.statusMessage = "Загрузка ленты...";
        refreshCurrentFeed();
        loading_ = false;
    });
}

void AppController::addSubscription(const std::string& url) {
    if (url.empty()) return;
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& u : appState_.subscriptions) {
        if (u == url) {
            session_.statusMessage = "Лента уже добавлена!";
            return;
        }
    }
    
    appState_.subscriptions.push_back(url);
    store_->save(appState_);
    
    Feed f(url);
    f.title = url;
    feeds_.push_back(f);
    session_.statusMessage = "Лента добавлена: " + url;
}

void AppController::removeSubscription(const std::string& url) {
    if (url.empty()) return;
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto& subs = appState_.subscriptions;
    subs.erase(std::remove(subs.begin(), subs.end(), url), subs.end());
    
    feeds_.erase(std::remove_if(feeds_.begin(), feeds_.end(),
        [&url](const Feed& f) { return f.url == url; }), feeds_.end());
    
    store_->save(appState_);
    session_.statusMessage = "Лента удалена: " + url;
}

void AppController::markRead(const std::string& articleId) {
    std::lock_guard<std::mutex> lock(mutex_);
    appState_.readFlags[articleId] = true;
    store_->save(appState_);
}

std::string AppController::fetchFullArticle(const std::string& articleId) {
    std::string url;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& feed : feeds_) {
            if (feed.url == session_.currentFeedUrl) {
                for (const auto& art : feed.articles) {
                    if (art.id == articleId) {
                        url = art.link;
                        break;
                    }
                }
            }
        }
    }
    
    if (url.empty()) return "Ссылка на статью не найдена.";

    session_.statusMessage = "Загрузка полной статьи с сайта...";
    LoadResult res = loader_->load(url);
    if (res.body.empty()) return "Ошибка загрузки страницы (возможно, блокировка).";

    std::string html = res.body;
    std::string text_to_render = html;

    std::string start_tag = "article-formatted-body";
    size_t start_pos = html.find(start_tag);
    
    if (start_pos != std::string::npos) {
        size_t content_start = html.find('>', start_pos);
        size_t content_end = html.find("</div>", content_start);
        
        if (content_start != std::string::npos && content_end != std::string::npos) {
            text_to_render = html.substr(content_start + 1, content_end - content_start - 1);
        }
    } else {
        session_.statusMessage = "Тег статьи не найден, показываю всю страницу.";
    }

    HtmlRenderer renderer;
    return renderer.render(text_to_render, 80);
}

void AppController::loadKeybindings(const std::string& path) {
    std::ifstream file(path);
    if (!file) return;

    std::string line;
    while (std::getline(file, line)) {
        auto hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        
        auto eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            
            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t\r\n"));
                s.erase(s.find_last_not_of(" \t\r\n") + 1);
            };
            trim(key); trim(val);
            
            if (!key.empty() && !val.empty()) {
                keybindings_[key] = val;
            }
        }
    }
}

std::string AppController::getKey(const std::string& action) const {
    auto it = keybindings_.find(action);
    if (it != keybindings_.end()) return it->second;
    
    if (action == "quit") return "q";
    if (action == "reload") return "r";
    if (action == "help") return "?";
    if (action == "back") return "Escape";
    if (action == "open") return "Return";
    return "";
}

} // namespace rss