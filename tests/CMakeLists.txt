add_executable(rss_tests core_tests.cpp)
target_link_libraries(rss_tests PRIVATE rss_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(rss_tests)

# Копируем папку testdata в build при сборке
file(COPY ${CMAKE_SOURCE_DIR}/testdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})