
cmake_minimum_required(VERSION 3.0)

include(CMakeParseArguments)

function(preinstall_dependency name sourceDir)
	set(depBaseDir ${CMAKE_BINARY_DIR}/__dependency__/${name})
	set(buildDir ${depBaseDir}/build)
	set(installDir ${depBaseDir}/install)

	set(buildTypes "Debug" "Release")
	if(CMAKE_BUILD_TYPE)
		list(REMOVE_ITEM buildTypes ${CMAKE_BUILD_TYPE})
		list(APPEND buildTypes ${CMAKE_BUILD_TYPE})
	endif()

	# set arguments
	cmake_parse_arguments("DEP" "" "" "CONFIG_ARGS;BUILD_ARGS" ${ARGN})

	# preapre a package
	file(MAKE_DIRECTORY ${installDir} ${buildDir})

	message(STATUS "Preparing '${name}' : ${sourceDir}")
	foreach(buildType ${buildTypes})
		# config a package
		exec_program(${CMAKE_COMMAND}
			ARGS
				"\"${sourceDir}\""
				"-B\"${buildDir}\""
				"-G\"${CMAKE_GENERATOR}\""
				"-DCMAKE_INSTALL_PREFIX=\"${installDir}\""
				"-DCMAKE_BUILD_TYPE=${buildType}"
				${DEP_CONFIG_ARGS}
			OUTPUT_VARIABLE stdout
			RETURN_VALUE result
		)
		if(NOT ${result} EQUAL 0)
			message(FATAL_ERROR
				"Failed to configure '${name}' ${buildType} :\n"
				"${stdout}"
			)
		endif()

		# build a package
		exec_program(${CMAKE_COMMAND}
			ARGS
				"--build" "\"${buildDir}\""
				${DEP_BUILD_ARGS}
				"--config" "${buildType}"
				"--target" "install"
			OUTPUT_VARIABLE stdout
			RETURN_VALUE result
		)
		if(NOT ${result} EQUAL 0)
			message(FATAL_ERROR
				"Failed to build '${name}' ${buildType} :\n"
				"${stdout}"
			)
		endif()
	endforeach()

	# config & build a package in build-time
	set(configDefaultArgs
		"${sourceDir}"
		"-B" "${buildDir}"
		"-G" "${CMAKE_GENERATOR}"
		"-DCMAKE_INSTALL_PREFIX=${installDir}"
	)
	if(CMAKE_BUILD_TYPE)
		list(APPEND configDefaultArgs "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
	endif()

	add_custom_target(${name}-config ALL
		${CMAKE_COMMAND}
			${configDefaultArgs}
			${DEP_CONFIG_ARGS}
		COMMENT "Configuring '${name}' : ${sourceDir}"
	)
	add_custom_target(${name}-build ALL
		${CMAKE_COMMAND}
			"--build" "${buildDir}"
			${DEP_BUILD_ARGS}
			"--config" "$<CONFIG>"
			"--target" "install"
		DEPENDS ${name}-config
		COMMENT "Building '${name}' : ${sourceDir}"
	)

	# set result dir
	message(STATUS "Set '${name}_ROOT' : ${installDir}")
	set(${name}_ROOT ${installDir} PARENT_SCOPE)
endfunction()
