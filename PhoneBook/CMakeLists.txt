cmake_minimum_required(VERSION 3.16)

project(PhoneBook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

qt_standard_project_setup()

qt_add_executable(PhoneBook
    main.cpp
    mainwindow.h
    mainwindow.cpp
    database.h
    database.cpp
)

target_link_libraries(PhoneBook
    PRIVATE
    Qt6::Widgets
    Qt6::Sql
)