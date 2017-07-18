
cmake_minimum_required(VERSION 3.1)

project(library)

# library for developing modules on middleware
find_package(BiCOMC CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

file(GLOB_RECURSE sources src/* include/*)
add_library(${PROJECT_NAME} STATIC ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES
	OUTPUT_NAME ${CMAKE_PROJECT_NAME}
	POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		-DRAPIDJSON_HAS_STDSTRING
)
target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	PRIVATE
		${RAPIDJSON_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:BiCOMC::BiCOMC>
)
if(WIN32)
	target_link_libraries(${PROJECT_NAME}
		PRIVATE Ws2_32
	)
else()
	target_link_libraries(${PROJECT_NAME}
		PRIVATE pthread
	)
endif()

# byname library
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
target_link_libraries(${CMAKE_PROJECT_NAME} INTERFACE ${PROJECT_NAME})

# install
include(${CMAKE_SOURCE_DIR}/cmake/install_interface_includes.cmake)

install(TARGETS ${PROJECT_NAME} ${CMAKE_PROJECT_NAME}
	EXPORT ${TARGET_EXPORT_NAME}
	RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR}
	LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
	ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR}
	INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
)
install_includes(include ${INCLUDE_INSTALL_DIR})

install_interface_includes(BiCOMC::BiCOMC ${INCLUDE_INSTALL_DIR})
