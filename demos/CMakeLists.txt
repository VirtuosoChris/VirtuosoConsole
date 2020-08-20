cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(VirtuosoConsole)

add_executable(ConsoleTest consoleTest.cpp ../QuakeStyleConsole.h)

add_custom_command(TARGET ConsoleTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_SOURCE_DIR}/TestCommands.txt $<TARGET_FILE_DIR:ConsoleTest>)

add_custom_command(TARGET ConsoleTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_SOURCE_DIR}/file2.txt $<TARGET_FILE_DIR:ConsoleTest>)