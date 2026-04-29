include(FetchContent)

set(CPR_USE_SYSTEM_CURL ON CACHE BOOL "" FORCE)
set(CPR_ENABLE_LIBPSL OFF CACHE INTERNAL "")

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG 1.12.0)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.12.0)

FetchContent_MakeAvailable(json cpr)

add_executable(${PROJECT_NAME} 
    main.cpp
    app.cpp
    ip_client.cpp
    forecast.cpp
    places.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json cpr::cpr)