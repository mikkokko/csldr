cmake_minimum_required(VERSION 3.16)

project(csldr)

if (MSVC)
	# /MT or /MTd on debug
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	if (NOT DEBUG_BUILD)
		add_compile_options(/arch:SSE2 /fp:fast)
	endif()
else()
	if (NOT NO_FORCE_32BIT)
		add_compile_options(-m32)
		add_link_options(-m32)
	endif()

	add_compile_options(-Wall -Wextra)

	if (NOT DEBUG_BUILD)
		add_compile_options(-Ofast -msse -msse2 -mfpmath=sse)
	endif()

	if (WIN32)
		add_link_options(-static -s)
	endif()
endif()

set(SRCDIR ${CMAKE_CURRENT_LIST_DIR})

#
# csldr
#

file(GLOB CSLDR_SRC ${SRCDIR}/csldr/*.c)
file(GLOB CSLDR_HDR ${SRCDIR}/csldr/*.h)

if (MSVC AND MSVC_VERSION LESS 1800)
	message("MSVC Version ${MSVC_VERSION}, no C99 support, compiling as C++")
	set_source_files_properties(${CSLDR_SRC} PROPERTIES LANGUAGE CXX)
endif()

add_library(csldr SHARED
	${CSLDR_SRC}
	${CSLDR_HDR})

target_include_directories(csldr PRIVATE ${SRCDIR}/external/sdk)

target_sources(csldr PRIVATE ${SRCDIR}/external/glad/src/glad.c)
target_include_directories(csldr PRIVATE ${SRCDIR}/external/glad/include ${SRCDIR}/shaders)

if (DEBUG_BUILD)
	# load shaders from a hardcoded directory on debug builds
	target_compile_definitions(csldr PRIVATE SHADER_DIR="${SRCDIR}/shaders")
endif()

target_precompile_headers(csldr PRIVATE ${SRCDIR}/csldr/pch.h)

set_target_properties(csldr PROPERTIES OUTPUT_NAME "client")
set_target_properties(csldr PROPERTIES PREFIX "")

if (APPLE)
	target_link_options(csldr PRIVATE -framework CoreFoundation)
endif()

if (OUTDIR)
	add_custom_command(TARGET csldr POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:csldr>
		${OUTDIR}/$<TARGET_FILE_NAME:csldr>)
endif()
