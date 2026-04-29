cmake_minimum_required(VERSION 3.20)

project(
    labwork9
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

set(CPR_USE_SYSTEM_CURL ON CACHE BOOL "" FORCE)
set(CPR_ENABLE_LIBPSL OFF CACHE INTERNAL "")

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG 1.12.0)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.12.0)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    FIND_PACKAGE_ARGS NAMES GTest
)

FetchContent_MakeAvailable(json cpr googletest)

add_executable(labwork9
    src/main.cpp
    src/app.cpp
    src/ip_client.cpp
    src/forecast.cpp
    src/places.cpp
)

target_include_directories(labwork9 PUBLIC labwork9)
target_link_libraries(labwork9 PRIVATE nlohmann_json::nlohmann_json cpr::cpr)

include_directories(lib)

enable_testing()
add_executable(
    scheduler-lib-tests
    tests/TTaskScheduler_ut.cpp
    tests/TTaskSchedulerCore_ut.cpp
    tests/app_ut.cpp
)
target_include_directories(scheduler-lib-tests PRIVATE include)
target_link_libraries(scheduler-lib-tests PRIVATE cpr::cpr nlohmann_json::nlohmann_json gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(scheduler-lib-tests)
