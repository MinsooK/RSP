
cmake_minimum_required(VERSION 3.1)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# hunter
set(HUNTER_USE_CACHE_SERVERS NO)
if(NOT "${HUNTER_ROOT}" AND NOT "$ENV{HUNTER_ROOT}")
	set(HUNTER_ROOT "${CMAKE_CURRENT_LIST_DIR}/dependency/hunter")
endif()
include(cmake/HunterGate.cmake)
HunterGate(
	URL "file://${CMAKE_CURRENT_LIST_DIR}/dependency/hunter/hunter-0.19.18.tar.gz"
	SHA1 "22b46b6d8c7645119c4dd5ed302eb387119f3283"
	FILEPATH "${CMAKE_CURRENT_LIST_DIR}/cmake/HunterConfig.cmake"
)

project(InduRoP VERSION 0.1.0)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

include(cmake/dependency.cmake)

############ install prefix normalization ############
file(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX)
string(TOLOWER ${CMAKE_INSTALL_PREFIX} installDir__)
if(NOT ${installDir__} MATCHES ".*${PROJECT_NAME_LOWER}.*")
	file(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX} installDir__)
	if(${installDir__} MATCHES ".*/$")
		string(APPEND installDir__ ${PROJECT_NAME})
	else()
		string(APPEND installDir__ "/" ${PROJECT_NAME})
	endif()
	set(CMAKE_INSTALL_PREFIX ${installDir__} CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
endif()
unset(installDir__)
############ install prefix normalization ############

################ install directories ################
set(RUNTIME_INSTALL_DIR     bin        CACHE PATH "Install dir for executables and dlls")
set(INCLUDE_INSTALL_DIR     include    CACHE PATH "Install dir for headers")
set(LIBRARY_INSTALL_DIR     lib        CACHE PATH "Install dir for shared libraries")
set(ARCHIVE_INSTALL_DIR     lib        CACHE PATH "Install dir for static libraries")
set(CONFIG_INSTALL_DIR      cmake      CACHE PATH "Install dir for cmake package config files")

set(VARIABLE_INSTALL_DIR    var        CACHE PATH "Install dir for shared variable definitions")
set(MODULE_INSTALL_DIR      module     CACHE PATH "Install dir for modules")

mark_as_advanced(
	RUNTIME_INSTALL_DIR INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR ARCHIVE_INSTALL_DIR
	VARIABLE_INSTALL_DIR MODULE_INSTALL_DIR CONFIG_INSTALL_DIR
)
################ install directories ################

set(TARGET_EXPORT_NAME ${PROJECT_NAME_LOWER}-target)
set(CMAKE_DEBUG_POSTFIX d)

################ list of subprojects ################
add_subdirectory(library) # for developing modules on middleware

add_subdirectory(ivgen)
add_subdirectory(middleware)
add_subdirectory(debugger)
add_subdirectory(io-dispatcher)
add_subdirectory(event-handler)
add_subdirectory(examples)
################ list of subprojects ################

add_custom_target(uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/cmake/uninstall.cmake"
)

############ generating package config files ############
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PROJECT_NAME_LOWER}-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)
configure_package_config_file(cmake/config.cmake.in ${PROJECT_NAME_LOWER}-config.cmake
	INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
	PATH_VARS
		RUNTIME_INSTALL_DIR INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR ARCHIVE_INSTALL_DIR
		VARIABLE_INSTALL_DIR MODULE_INSTALL_DIR CONFIG_INSTALL_DIR
)
install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config.cmake
	DESTINATION ${CONFIG_INSTALL_DIR}
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
)

install(EXPORT ${TARGET_EXPORT_NAME}
	NAMESPACE ${PROJECT_NAME}::
	DESTINATION ${CONFIG_INSTALL_DIR}
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
)
############ generating package config files ############

export(EXPORT ${TARGET_EXPORT_NAME}
	NAMESPACE ${PROJECT_NAME}::
)
