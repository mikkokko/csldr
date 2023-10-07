cmake_minimum_required(VERSION 3.16)

project(csldr C)

if (NOT DEBUG_BUILD)
	set(UNITY_BUILD ON)
endif()

if (UNITY_BUILD)
	set(CMAKE_UNITY_BUILD_BATCH_SIZE 128)
endif()

if (MSVC)
	# /MT or /MTd on debug
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	add_compile_options(/W4 /wd4201)
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
	add_compile_options(-m32 -Wall -Wextra)
	add_link_options(-m32)
	
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

add_library(csldr SHARED
	${CSLDR_SRC}
	${CSLDR_HDR})

if (UNITY_BUILD)
	set_target_properties(csldr PROPERTIES UNITY_BUILD ON)
endif()

target_include_directories(csldr PRIVATE ${SRCDIR}/external/sdk)

target_sources(csldr PRIVATE ${SRCDIR}/external/glad/src/glad.c)
target_include_directories(csldr PRIVATE ${SRCDIR}/external/glad/include ${SRCDIR}/shaders)

if (DEBUG_BUILD)
	# load shaders from a hardcoded directory on debug builds
	target_compile_definitions(csldr PRIVATE SHADER_DIR="${SRCDIR}/shaders")
endif()

if (MSVC AND DEBUG_BUILD)
	set_source_files_properties(${CSLDR_SRC} PROPERTIES COMPILE_FLAGS "/Yupch.h")
	set_source_files_properties(${SRCDIR}/csldr/pch.c PROPERTIES COMPILE_FLAGS "/Ycpch.h")
endif()

set_target_properties(csldr PROPERTIES OUTPUT_NAME "client")
set_target_properties(csldr PROPERTIES PREFIX "")

if (OUTDIR)
	add_custom_command(TARGET csldr POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:csldr>
		${OUTDIR}/$<TARGET_FILE_NAME:csldr>)
endif()
