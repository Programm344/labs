cmake_minimum_required(VERSION 3.10)
project(cpp_crow_lab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Название исполняемого файла
set(TARGET_NAME cpp_info_api)

# ----------------------------------------------------------------------------
# ПОДКЛЮЧЕНИЕ Crow
# ----------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(crow)

# ----------------------------------------------------------------------------
# ПОИСК ОСТАЛЬНЫХ БИБЛИОТЕК
# ----------------------------------------------------------------------------
find_package(Boost REQUIRED COMPONENTS system)
find_package(SQLite3 REQUIRED)

# ----------------------------------------------------------------------------
# ПОИСК .cpp ФАЙЛОВ
# ----------------------------------------------------------------------------
file(GLOB_RECURSE SOURCES "src/*.cpp")

# ----------------------------------------------------------------------------
# СОЗДАНИЕ ЦЕЛИ
# ----------------------------------------------------------------------------
add_executable(${TARGET_NAME} ${SOURCES})

# ----------------------------------------------------------------------------
# ПОДКЛЮЧЕНИЕ БИБЛИОТЕК (ВАЖНО: используем Crow::Crow, а не просто crow)
# ----------------------------------------------------------------------------
target_link_libraries(${TARGET_NAME}
    Crow::Crow          # ← правильный способ!
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
)

# ----------------------------------------------------------------------------
# ДОБАВЛЕНИЕ ДИРЕКТОРИЙ
# ----------------------------------------------------------------------------
target_include_directories(${TARGET_NAME} PRIVATE
    ${SQLite3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Опции компиляции
target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra)

message(STATUS "Project configured successfully!")
