add_library(rss_app STATIC
    AppController.cpp
)
target_link_libraries(rss_app PUBLIC rss_core)
target_include_directories(rss_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})