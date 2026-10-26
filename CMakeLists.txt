cmake_minimum_required(VERSION 3.20)
project(rss_reader CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Werror)

include(FetchContent)
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# --- Добавляем папки проекта ---
find_package(CURL REQUIRED)     # <-- CURL нужен здесь
add_subdirectory(src/core)      # <-- ЭТОЙ СТРОКИ У ТЕБЯ НЕ ХВАТАЕТ!
add_subdirectory(tests)
add_subdirectory(src/app)
# --- Собираем исполняемый файл ---
add_executable(rss_reader src/tui/main.cpp)
target_link_libraries(rss_reader PRIVATE 
    rss_app 
    rss_core 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
)
