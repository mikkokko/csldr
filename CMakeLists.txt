cmake_minimum_required(VERSION 3.0)

project(csldr)

if (MSVC)
    add_compile_options(/W4 /WX)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    set(CMAKE_C_FLAGS "-m32")
	add_compile_options(-std=c89 -pedantic -Wall -Wextra -Werror)
endif()

set(SRCDIR ${CMAKE_CURRENT_LIST_DIR})
set(GAMEDIR ${CMAKE_CURRENT_LIST_DIR}/../game)

#
# csldr
#

file(GLOB CSLDR_SRC ${SRCDIR}/csldr/*.c )
file(GLOB CSLDR_HDR ${SRCDIR}/csldr/*.h)

file(GLOB SDK_SRC ${SRCDIR}/sdk/*.c)
file(GLOB SDK_HDR ${SRCDIR}/sdk/*.h)

add_library(csldr SHARED ${CSLDR_SRC} ${CSLDR_HDR} ${SDK_SRC} ${SDK_HDR})

if (MSVC)
	foreach(src ${CSLDR_SRC})
		set_source_files_properties(${src} PROPERTIES COMPILE_FLAGS "/Yupch.h")			
    endforeach(src ${CSLDR_SRC})
	
	set_source_files_properties(${SRCDIR}/csldr/pch.c PROPERTIES COMPILE_FLAGS "/Ycpch.h")
endif()

if (WIN32)
	# is linking opengl sus
	target_link_libraries(csldr opengl32)
endif()

set_target_properties(csldr PROPERTIES OUTPUT_NAME  "client")
set_target_properties(csldr PROPERTIES PREFIX "")

add_custom_command(TARGET csldr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:csldr> ${GAMEDIR}/cstrike/cl_dlls/$<TARGET_FILE_NAME:csldr>)
