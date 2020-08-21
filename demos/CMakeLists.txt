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

find_package(OpenGL REQUIRED)

add_executable(GuiTest guiTest.cpp 
../QuakeStyleConsole.h
../IMGUIQuakeConsole.h
Depends/imgui/imgui.cpp 
Depends/imgui/imgui_widgets.cpp
Depends/imgui/imgui_draw.cpp
)

target_include_directories(GuiTest PUBLIC "Depends")
target_include_directories(GuiTest PUBLIC "Depends/imgui")
target_include_directories(GuiTest PUBLIC "Depends/glfw/include")
target_include_directories(GuiTest PUBLIC "Depends/glhpp")

target_link_libraries(GuiTest OpenGL::GL)


if (${APPLE})
    target_link_libraries(GuiTest "-framework Cocoa")
    target_link_libraries(GuiTest "-framework IoKit")
endif()

add_subdirectory(Depends/glfw)
target_link_libraries(GuiTest glfw)

