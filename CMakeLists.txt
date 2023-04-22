cmake_minimum_required(VERSION 3.15)

project(csldr C)

if (MSVC)
	# /MT or /MTd on debug
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	add_compile_options(/W4)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

	# most of these are enabled implicitly but we set them anyway
	if (NOT DEBUG_BUILD)
		add_compile_options(
			/O2
			/Ob2
			/Oi
			/Ot
			/Oy
			/GL
			/GS-
			/Gy
			/Gw
			/arch:SSE2 # x86
			/fp:fast
			/fp:except-)

		add_link_options(
			/OPT:REF
			/OPT:ICF
			/LTCG)
	endif()
else()
	# just use -march=native, everyone should build this themselves
	add_compile_options(-m32 -Wall -Wextra -Ofast -march=native)
	add_link_options(-m32)
endif()

set(SRCDIR ${CMAKE_CURRENT_LIST_DIR})

#
# csldr
#

file(GLOB CSLDR_SRC ${SRCDIR}/csldr/*.c)
file(GLOB CSLDR_HDR ${SRCDIR}/csldr/*.h)

add_library(csldr SHARED
	${CSLDR_SRC}
	${CSLDR_HDR}
	${SRCDIR}/external/glad/src/glad.c)

target_include_directories(csldr PRIVATE
	${SRCDIR}/external/glad/include
	${SRCDIR}/external/sdk
	${SRCDIR}/shaders)

if (MSVC)
	set_source_files_properties(${CSLDR_SRC} PROPERTIES COMPILE_FLAGS "/Yupch.h")
	set_source_files_properties(${SRCDIR}/csldr/pch.c PROPERTIES COMPILE_FLAGS "/Ycpch.h")
endif()

if (WIN32)
	# still needed for wglGetCurrentContext...
	target_link_libraries(csldr PRIVATE opengl32)
endif()

set_target_properties(csldr PROPERTIES OUTPUT_NAME "client")
set_target_properties(csldr PROPERTIES PREFIX "")

if (OUTDIR)
	add_custom_command(TARGET csldr POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:csldr>
		${OUTDIR}/$<TARGET_FILE_NAME:csldr>)
endif()
