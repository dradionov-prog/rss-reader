add_library(rss_core STATIC
    Rss2Parser.cpp
    AtomParser.cpp
)

target_include_directories(rss_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rss_core PUBLIC cxx_std_20)
target_link_libraries(rss_core PUBLIC pugixml::pugixml)