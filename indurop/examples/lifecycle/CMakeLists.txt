
cmake_minimum_required(VERSION 3.1)

get_filename_component(projectName__ ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "-" projectName__ ${projectName__})
project(${projectName__})
unset(projectName__)

if(NOT TARGET InduRoP::InduRoP)
	find_package(InduRoP CONFIG REQUIRED)
endif()

add_library(${PROJECT_NAME} SHARED main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE InduRoP::InduRoP)
