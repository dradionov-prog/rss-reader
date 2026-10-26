#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include "Feed.hpp"
#include "StateStore.hpp"
#include "ParserRegistry.hpp"
#include "FeedLoader.hpp"
#include "HtmlRenderer.hpp"

namespace rss {

enum class Screen { FeedList, ArticleList, ArticleView, Help };

struct SessionState {
    Screen currentScreen = Screen::FeedList;
    std::string currentFeedUrl;
    std::string currentArticleId;
    std::string statusMessage;
};

class AppController {
public:
    AppController(std::shared_ptr<StateStore> store, 
                  std::shared_ptr<ParserRegistry> parsers,
                  std::shared_ptr<FeedLoader> loader);
    ~AppController();

    void reloadData();
    void refreshCurrentFeed();
    void refreshInBackground();
    bool isLoading() const { return loading_; }
    
    void addSubscription(const std::string& url);
    void removeSubscription(const std::string& url); // <-- НОВЫЙ МЕТОД
    void markRead(const std::string& articleId);
    
    std::string fetchFullArticle(const std::string& articleId);

    void loadKeybindings(const std::string& path);
    std::string getKey(const std::string& action) const;

    const std::vector<Feed>& getFeeds() const { 
        std::lock_guard<std::mutex> lock(mutex_); 
        return feeds_; 
    }
    
    const AppState& getState() const { 
        std::lock_guard<std::mutex> lock(mutex_); 
        return appState_; 
    }
    
    SessionState& getSession() { return session_; }

private:
    std::shared_ptr<StateStore> store_;
    std::shared_ptr<ParserRegistry> parsers_;
    std::shared_ptr<FeedLoader> loader_;
    
    AppState appState_;
    SessionState session_;
    std::vector<Feed> feeds_;
    std::map<std::string, std::string> keybindings_;

    std::atomic<bool> loading_{false};
    std::thread worker_;
    mutable std::mutex mutex_;
};

} // namespace rss