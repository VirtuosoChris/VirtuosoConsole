add_executable(ConsoleTest main.cpp)

if(Boost_FOUND)
include_directories(${Boost_INCLUDE_DIRS} ${BOOST_INCLUDE_PATH} ../)
endif()

target_link_libraries(ConsoleTest VirtuosoConsole)
