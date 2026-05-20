cmake_minimum_required(VERSION 3.16)
project(FitnessClub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Путь к Qt
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.3/msvc2022_64")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    main.cpp
    MainWindow.cpp
    FitnessClubSystem.cpp
)

set(HEADERS
    MainWindow.h
    FitnessClubSystem.h
    Repository.h
)

add_executable(FitnessClub ${SOURCES} ${HEADERS})

target_link_libraries(FitnessClub Qt6::Core Qt6::Widgets)

# Включаем русскую кодировку
if(MSVC)
    target_compile_options(FitnessClub PRIVATE /utf-8)
endif()

# КОПИРУЕМ database.txt В ПАПКУ С ПРОГРАММОЙ
add_custom_command(TARGET FitnessClub POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/database.txt
        $<TARGET_FILE_DIR:FitnessClub>/database.txt
)