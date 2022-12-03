cmake_minimum_required(VERSION 3.0)

project(csldr C)

set(OUTDIR CACHE STRING "Directory to output library to")

if (MSVC)
	add_compile_options(/W4 /wd4244)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
	add_compile_options(-m32 -Wall -Wextra)
	add_link_options(-m32)
endif()

set(SRCDIR ${CMAKE_CURRENT_LIST_DIR})

#
# csldr
#

file(GLOB CSLDR_SRC ${SRCDIR}/csldr/*.c)
file(GLOB CSLDR_HDR ${SRCDIR}/csldr/*.h)

add_library(csldr SHARED ${CSLDR_SRC} ${CSLDR_HDR})

if (MSVC)
	foreach(src ${CSLDR_SRC})
		set_source_files_properties(${src} PROPERTIES COMPILE_FLAGS "/Yupch.h")
	endforeach(src ${CSLDR_SRC})

	set_source_files_properties(${SRCDIR}/csldr/pch.c PROPERTIES COMPILE_FLAGS "/Ycpch.h")
endif()

if (WIN32)
	target_link_libraries(csldr PRIVATE opengl32)
endif()

set_target_properties(csldr PROPERTIES OUTPUT_NAME "client")
set_target_properties(csldr PROPERTIES PREFIX "")

# from csgo-lite
if (OUTDIR)
	add_custom_command(TARGET csldr POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:csldr>
		${OUTDIR}/$<TARGET_FILE_NAME:csldr>)
endif()
