cmake_minimum_required(VERSION 3.21.1)

project(cShell)

add_executable(${PROJECT_NAME} cShell.c)

target_link_libraries(${PROJECT_NAME} readline)
