
macro(config_compiler_and_linker)
	if (MSVC)
		set(cxx_base_flags "-GS -W4 -WX -wd4251 -wd4275 -nologo -J -Zi")
		if (MSVC_VERSION LESS 1400) # 1400 is Visual Studio 2005
			set(cxx_base_flags "${cxx_base_flags} -wd4800")
			set(cxx_base_flags "${cxx_base_flags} -wd4511 -wd4512")
			set(cxx_base_flags "${cxx_base_flags} -wd4675")
		endif()
		if (MSVC_VERSION LESS 1500) # 1500 is Visual Studio 2008
			set(cxx_base_flags "${cxx_base_flags} -wd4127")
		endif()
		if (NOT (MSVC_VERSION LESS 1700)) # 1700 is Visual Studio 2012.
			set(cxx_base_flags "${cxx_base_flags} -wd4702")
		endif()
	
		set(cxx_base_flags "${cxx_base_flags} -D_UNICODE -DUNICODE -DWIN32 -D_WIN32")
		set(cxx_base_flags "${cxx_base_flags} -DSTRICT -DWIN32_LEAN_AND_MEAN")
		set(cxx_exception_flags "-EHsc -D_HAS_EXCEPTIONS=1")
	elseif (CMAKE_COMPILER_IS_GNUCXX)
		set(cxx_base_flags "-Wall -Wshadow -Werror")
		if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0.0)
			set(cxx_base_flags "${cxx_base_flags} -Wno-error=dangling-else")
		endif()
		set(cxx_exception_flags "-fexceptions")

		set(cxx_strict_flags "-Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
	endif()
	
	set(cxx_exception "${cxx_base_flags} ${cxx_exception_flags}")
	set(cxx_default "${cxx_exception}")

	set(cxx_strict "${cxx_default} ${cxx_strict_flags}")
endmacro()

function(cxx_library_with_type name type cxx_flags)
	add_library(${name} ${type} ${ARGN})
	set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${cxx_flags}")
	set_target_properties(${name} PROPERTIES DEBUG_POSTFIX "d")
endfunction()

function(cxx_library name cxx_flags)
	cxx_library_with_type(${name} "" "${cxx_flags}" ${ARGN})
endfunction()

######## cxx_executable_with_flags(name cxx_flags libs srcs...)
function(cxx_executable_with_flags name cxx_flags libs)
	add_executable(${name} ${ARGN})
	if (MSVC AND (NOT (MSVC_VERSION LESS 1700))) # 1700 is Visual Studio 2012.
		# BigObj required for tests.
		set(cxx_flags "${cxx_flags} -bigobj")
	endif()
	if (cxx_flags)
		set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${cxx_flags}")
	endif()
	foreach (lib "${libs}")
		target_link_libraries(${name} ${lib})
	endforeach()
endfunction()

######## cxx_executable(name dir lib srcs...)
function(cxx_executable name dir libs)
	cxx_executable_with_flags(${name} "${cxx_default}" "${libs}" "${dir}/${name}.c" ${ARGN})
endfunction()
