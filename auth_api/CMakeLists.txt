cmake_minimum_required(VERSION 3.10)
project(auth_api)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ----------------------------------------------------------------------
# Crow (header-only)
# ----------------------------------------------------------------------
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.0+5
)

# ----------------------------------------------------------------------
# JSON библиотека
# ----------------------------------------------------------------------
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

# ----------------------------------------------------------------------
# JWT библиотека (правильный репозиторий)
# ----------------------------------------------------------------------
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG master
)

# ----------------------------------------------------------------------
# bcrypt библиотека
# ----------------------------------------------------------------------
FetchContent_Declare(
    bcrypt
    GIT_REPOSITORY https://github.com/trusch/libbcrypt.git
    GIT_TAG master
)

# ----------------------------------------------------------------------
# Скачиваем всё
# ----------------------------------------------------------------------
FetchContent_MakeAvailable(crow nlohmann_json jwt-cpp bcrypt)

# ----------------------------------------------------------------------
# Системные библиотеки
# ----------------------------------------------------------------------
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# ----------------------------------------------------------------------
# Находим все .cpp файлы
# ----------------------------------------------------------------------
file(GLOB_RECURSE SOURCES "src/*.cpp")

# ----------------------------------------------------------------------
# Создаём исполняемый файл
# ----------------------------------------------------------------------
add_executable(auth_api ${SOURCES})

# ----------------------------------------------------------------------
# Подключаем заголовочные файлы (ВАЖНО: добавляем все пути)
# ----------------------------------------------------------------------
target_include_directories(auth_api PRIVATE
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    src
    ${nlohmann_json_SOURCE_DIR}/include
    ${jwt-cpp_SOURCE_DIR}/include      # ← путь к jwt-cpp
    ${bcrypt_SOURCE_DIR}/include        # ← путь к bcrypt
)

# ----------------------------------------------------------------------
# Линкуем библиотеки
# ----------------------------------------------------------------------
target_link_libraries(auth_api
    ${SQLite3_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    bcrypt
)

# ----------------------------------------------------------------------
# Опции компиляции
# ----------------------------------------------------------------------
target_compile_options(auth_api PRIVATE -Wall -Wextra)

message(STATUS "Project configured successfully")
