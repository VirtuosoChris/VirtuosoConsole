cmake_minimum_required(VERSION 2.8)

project(VirtuosoConsole)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wpedantic")

#boost stuff.  should auto fail if boost not found
find_package(Boost 1.46.0)


if(Boost_FOUND)
	message(STATUS "Boost found, console will be built with HISTORY_FILE defined")
	add_definitions( -DHISTORY_FILE )
	include_directories(${Boost_INCLUDE_DIRS})
else()
	 message(WARNING "Boost not found: console will not be built with circular buffer (history file) support")
endif()


set(SOURCESLIB 
GameConsole.cpp
GameConsole.h
GameConsole.tcc
DynamicVariable.h
BaseLog.h
SimpleLog.h
)			

add_library(VirtuosoConsole ${SOURCESLIB})

add_subdirectory(test)
