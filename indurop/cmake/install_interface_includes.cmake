
function(install_includes directories destination)
	set(targetDirs)
	set(targetFiles)

	foreach(dir ${directories})
		file(GLOB children ${dir}/*)
		foreach(child ${children})
			if(IS_DIRECTORY ${child})
				list(APPEND targetDirs ${child})
			else()
				list(APPEND targetFiles ${child})
			endif()
		endforeach()
	endforeach()

	if(targetDirs)
		install(DIRECTORY ${targetDirs} DESTINATION ${destination}
			FILE_PERMISSIONS
				OWNER_READ GROUP_READ WORLD_READ
		)
	endif()
	if(targetFiles)
		install(FILES ${targetFiles} DESTINATION ${destination}
			PERMISSIONS
				OWNER_READ GROUP_READ WORLD_READ
		)
	endif()
endfunction()

function(install_interface_includes target destination)
	get_target_property(includeDirectories ${target} INTERFACE_INCLUDE_DIRECTORIES)
	install_includes("${includeDirectories}" ${destination})
endfunction()
