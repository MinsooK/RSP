
cmake_minimum_required(VERSION 3.1)

if(NOT TARGET InduRoP::InduRoP)
	message(FATAL_ERROR "'InduRoP::InduRoP' must be exist. Please use 'find_package(InduRoP)'.")
endif()

if(NOT InduRoP_RUNTIME_DIR)
	message(FATAL_ERROR "'InduRoP_RUNTIME_DIR' must be required. Please set it.")
endif()

if(NOT InduRoP_VARIABLE_DIR)
	message(FATAL_ERROR "'InduRoP_VARIABLE_DIR' must be required. Please set it.")
endif()

if(NOT InduRoP_IVGEN_PATH)
	set(InduRoP_IVGEN_PATH ${InduRoP_RUNTIME_DIR}/ivgen.py)
endif()

include(CMakeParseArguments)

function(InduRoP_add_variable sourceList includeList name)
	cmake_parse_arguments("SHV" "IN;OUT;INOUT" "" "PATHS" ${ARGN})

	if(SHV_IN)
		set(direction "IN")
	elseif(SHV_OUT)
		set(direction "OUT")
	else()
		set(direction "INOUT")
	endif()

	set(path)
	if(NOT SHV_PATHS)
		set(SHV_PATHS ${InduRoP_VARIABLE_DIR})
	endif()

	if(name MATCHES ".*\\.glb$")
		list(APPEND SHV_PATHS ${CMAKE_CURRENT_LIST_DIR})
	endif()

	foreach(dir ${SHV_PATHS})
		if(EXISTS "${dir}/${name}.ivd")
			set(path ${dir})
			break()
		elseif(EXISTS "${dir}/${name}")
			set(path ${dir})
			break()
		endif()
	endforeach()
	if(NOT path)
		message(FATAL_ERROR "'${name}' is not exist.(${ARGV})")
	endif()

	set(generatedPath ${CMAKE_CURRENT_BINARY_DIR}/generated)
	if(name MATCHES ".*\\.glb$")
		string(REPLACE ".glb" "" byname "${name}")
		set(cppPath ${byname}.cpp)
		set(hppPath ${byname}.h)
	else()
		set(cppPath ${name}.cpp)
		set(hppPath ${name}.h)
	endif()

	file(MAKE_DIRECTORY ${generatedPath}/${direction})

	set(ivgen ${InduRoP_IVGEN_PATH})
	set(ivgenArgs)
	if(InduRoP_IVGEN_PATH MATCHES ".*\\.py$")
		set(Python_ADDITIONAL_VERSIONS 3)
		find_package(PythonInterp 3 REQUIRED)
		set(ivgen "${PYTHON_EXECUTABLE}")
		list(APPEND ivgenArgs "${InduRoP_IVGEN_PATH}")
	endif()
	list(APPEND ivgenArgs "${name}" "-${direction}" "-dir=${path}")

	add_custom_command(
		OUTPUT
			${generatedPath}/${direction}/${cppPath}
			${generatedPath}/${direction}/${hppPath}
		COMMAND ${ivgen}
		ARGS ${ivgenArgs}
		WORKING_DIRECTORY ${generatedPath}
		DEPENDS "${path}/${name}"
	)

	set_source_files_properties(
		${generatedPath}/${direction}/${cppPath} ${generatedPath}/${direction}/${hppPath}
		PROPERTIES GENERATED TRUE
	)

	set(includes ${${includeList}})
	list(APPEND includes "${generatedPath}/${direction}")
	list(REMOVE_DUPLICATES includes)
	set(${includeList} ${includes} PARENT_SCOPE)

	set(sources ${${sourceList}})
	list(APPEND sources "${generatedPath}/${direction}/${cppPath}")
	list(APPEND sources "${generatedPath}/${direction}/${hppPath}")
	list(REMOVE_DUPLICATES sources)
	set(${sourceList} ${sources} PARENT_SCOPE)
endfunction()

function(add_shared_variables target)
	cmake_parse_arguments("SHV" "" "" "NAMES;FILES;PATHS" ${ARGN})

	if(NOT SHV_NAMES AND NOT SHV_FILES)
		return()
	endif()

	foreach(name ${SHV_NAMES} ${SHV_FILES})
		set(sources)
		set(includes)

		InduRoP_add_variable(sources includes ${name} PATHS ${SHV_PATHS})

		target_include_directories(${target} PRIVATE ${includes})
		target_sources(${target} PRIVATE ${sources} ${SHV_FILES})
	endforeach()

	target_link_libraries(${target} PRIVATE InduRoP::InduRoP)
endfunction()
