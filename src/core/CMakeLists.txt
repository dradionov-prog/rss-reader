add_library(rss_core STATIC
    Rss2Parser.cpp
    AtomParser.cpp
    HttpLoader.cpp
    HtmlHandlers.cpp
    HtmlRenderer.cpp
    JsonStateStore.cpp      # <-- 1. Добавили сюда
)

target_include_directories(rss_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rss_core PUBLIC cxx_std_20)

find_package(CURL REQUIRED)
# 2. Привязали nlohmann_json
target_link_libraries(rss_core PUBLIC pugixml::pugixml CURL::libcurl nlohmann_json::nlohmann_json)