cmake_minimum_required(VERSION 3.10)
project(cpp_crow_lab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Название исполняемого файла
set(TARGET_NAME cpp_info_api)

# Найти все .cpp файлы в src/
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Папка с заголовочными файлами
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Создать исполняемый файл
add_executable(${TARGET_NAME} ${SOURCES})

# Опции компиляции
target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra)

# Поиск Boost (нужен для Crow)
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${TARGET_NAME} PRIVATE ${Boost_LIBRARIES})

# Сообщение об успехе
message(STATUS "Project ${PROJECT_NAME} configured")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
# ===== ТЕСТЫ =====
enable_testing()

# Находим Google Test
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Создаем папку для тестов (если её нет)
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

# Создаем исполняемый файл для тестов
add_executable(run_tests ${TEST_SOURCES})

# Подключаем необходимые библиотеки
target_link_libraries(run_tests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    Threads::Threads
    ${Boost_LIBRARIES}
)

# Подключаем заголовочные файлы
target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# Флаги компиляции для тестов
target_compile_options(run_tests PRIVATE -Wall -Wextra)

# Добавляем тест в CTest
add_test(NAME controller_tests COMMAND run_tests)

message(STATUS "Tests configured: run_tests")
