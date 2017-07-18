
################################################################################
# Input Variables :
#  Xenomai_ROOT : root directory of xenomai
#  Xenomai_SKIN : xenomai skin, if empty then it set 'native'
# Output Variables:
#  Xenomai_INCLUDE_DIRS : directories of header files
#  Xenomai_LIBRARIES : libraries
#  Xenomai_DEFINITIONS : definitions
#  Xenomai_CFLAGS : result of 'xeno-config --cflags'
#  Xenomai_LDFLAGS : result of 'xeno-config --ldflags'
################################################################################

set(XENOMAI_SEARCH_PATHS)

if(Xenomai_ROOT)
	list(APPEND XENOMAI_SEARCH_PATHS ${Xenomai_ROOT})
endif()

list(APPEND XENOMAI_SEARCH_PATHS /usr/xenomai /usr/local/xenomai)

foreach(path ${XENOMAI_SEARCH_PATHS})
	list(APPEND XENOMAI_SEARCH_BIN_PATHS ${path}/bin)
endforeach()

find_program(XENOMAI_CONFIG NAMES xeno-config PATHS ${XENOMAI_SEARCH_BIN_PATHS})
if(NOT XENOMAI_CONFIG)
	if(Xenomai_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find 'xeno-config'.")
	else()
		return()
	endif()
endif()

if(NOT Xenomai_SKIN)
	set(Xenomai_SKIN native)
endif()

execute_process(COMMAND ${XENOMAI_CONFIG} --version
	RESULT_VARIABLE XENOMAI_CONFIG_RESULT
	OUTPUT_VARIABLE Xenomai_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND ${XENOMAI_CONFIG} "--skin=${Xenomai_SKIN}" --cflags
	RESULT_VARIABLE XENOMAI_CONFIG_RESULT
	OUTPUT_VARIABLE Xenomai_CFLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT XENOMAI_CONFIG_RESULT EQUAL 0)
	message(FATAL_ERROR "'xeno-config --cflags' failed.")
endif()

execute_process(COMMAND ${XENOMAI_CONFIG} "--skin=${Xenomai_SKIN}" --ldflags
	RESULT_VARIABLE XENOMAI_CONFIG_RESULT
	OUTPUT_VARIABLE Xenomai_LDFLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT XENOMAI_CONFIG_RESULT EQUAL 0)
	message(FATAL_ERROR "'xeno-config --ldflags' failed.")
endif()

set(Xenomai_INCLUDE_DIRS)
set(Xenomai_DEFINITIONS)
separate_arguments(options UNIX_COMMAND ${Xenomai_CFLAGS})
foreach(option ${options})
	if(option MATCHES "^-I.*")
		string(SUBSTRING ${option} 2 -1 directory)
		list(APPEND Xenomai_INCLUDE_DIRS "${directory}")
	elseif(option MATCHES "^-.*")
		list(APPEND Xenomai_DEFINITIONS "${option}")
	else()
		message(WARNING "Mssing xenomai option : ${option}")
	endif()
endforeach()


set(Xenomai_LIBRARIES)
set(XENOMAI_LIBRARY_NAMES)
set(XENOMAI_LIBRARY_DIRS)
separate_arguments(options UNIX_COMMAND ${Xenomai_LDFLAGS})
foreach(option ${options})
	if(option MATCHES "^-l.*")
		string(SUBSTRING ${option} 2 -1 library)
		list(APPEND XENOMAI_LIBRARY_NAMES "${library}")
	elseif(option MATCHES "^-L.*")
		string(SUBSTRING ${option} 2 -1 directory)
		list(APPEND XENOMAI_LIBRARY_DIRS "${directory}")
	elseif(option MATCHES "^-.*")
		list(APPEND Xenomai_LIBRARIES "${option}")
	else()
		list(APPEND XENOMAI_LIBRARY_NAMES "${option}")
	endif()
endforeach()

foreach(library ${XENOMAI_LIBRARY_NAMES})
	set(fullpath "FOUND_LIB-NOTFOUND")
	find_library(fullpath NAMES ${library}
		PATHS ${XENOMAI_LIBRARY_DIRS}
		NO_DEFAULT_PATH
	)
	if(fullpath)
		list(APPEND Xenomai_LIBRARIES "${fullpath}")
	else()
		list(APPEND Xenomai_LIBRARIES "${library}")
	endif()
endforeach()

list(REMOVE_DUPLICATES Xenomai_INCLUDE_DIRS)
list(REMOVE_DUPLICATES Xenomai_LIBRARIES)
list(REMOVE_DUPLICATES Xenomai_DEFINITIONS)
set(Xenomai_FOUND true)
