cmake_minimum_required(VERSION 3.10)
project(AuthAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Crow для HTTP сервера
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.0+5
)

# JSON библиотека
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(crow nlohmann_json)

add_executable(auth_api 
    src/main.cpp
)

target_link_libraries(auth_api 
    crow 
    nlohmann_json::nlohmann_json
)

# Включаем предупреждения
target_compile_options(auth_api PRIVATE -Wall -Wextra)
