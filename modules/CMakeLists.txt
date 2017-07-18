
cmake_minimum_required(VERSION 3.1)

project(InduRoP-Modules)

# link middleware
find_package(InduRoP CONFIG REQUIRED)
# link orocos_kdl
INCLUDE_DIRECTORIES(${PROJ_SOURCE_DIR}/src ${PROJ_SOURCE_DIR}/models ${PROJ_BINARY_DIR}/src)
list(APPEND CMAKE_INCLUDE_PATH "/usr/local/include/")
find_package(orocos_kdl REQUIRED)
# link eigen
include_directories (/usr/local/include/eigen3/)



# module1
add_library(module1 SHARED module1.cpp)
add_shared_variables(module1 FILES monitor.glb)

# module2
add_library(module2 SHARED module2.cpp)
add_shared_variables(module2 FILES monitor.glb)
