
cmake_minimum_required(VERSION 3.1)

if(TARGET ${CMAKE_PROJECT_NAME})
	# this code is used by examples before 'InduRoP' installed.
	# please use 'find_package(InduRoP)' after 'InduRoP' installed.
	set(${CMAKE_PROJECT_NAME}_RUNTIME_DIR      "${CMAKE_BINARY_DIR}")
	set(${CMAKE_PROJECT_NAME}_VARIABLE_DIR     "${CMAKE_SOURCE_DIR}/var")
	set(${CMAKE_PROJECT_NAME}_IVGEN_PATH       "${CMAKE_SOURCE_DIR}/ivgen/ivgen.py")

	add_library(${CMAKE_PROJECT_NAME}::${CMAKE_PROJECT_NAME} ALIAS ${PROJECT_NAME})
	include(${CMAKE_SOURCE_DIR}/cmake/add_shared_variables.cmake)
endif()

# example projects
project(examples)

file(GLOB children ${CMAKE_CURRENT_LIST_DIR}/*)
foreach(child ${children})
	if(IS_DIRECTORY ${child})
		add_subdirectory(${child})
	endif()
endforeach()
