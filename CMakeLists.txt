cmake_minimum_required(VERSION 3.16)
project(DecayLib CXX)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

cmake_policy(SET CMP0077 NEW) # set() overrides option()

#--------------------------------
# Options
#--------------------------------

add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>") # Define DEBUG in Debug builds

# Takes more time to process but decreases Vertex array size.
# There can still be position duplicates as they would have different texture coordinates
option(DECAY_BSP_NO_DUPLICATES "Avoid duplicate vertices when parsing BSP" ON)
if(DECAY_BSP_NO_DUPLICATES)
    add_compile_definitions("BSP_NO_DUPLICATES")
endif()

# Still processes faces to triangles so they must be processed back (not slow).
option(DECAY_OBJ_POLYGONS "Exporting as OBJ uses polygons instead of triangles" ON)
if(DECAY_OBJ_POLYGONS)
    add_compile_definitions("BSP_OBJ_POLYGONS")
endif()

#--------------------------------
# Compiler configuration
#--------------------------------

if(MINGW)
    ADD_COMPILE_OPTIONS(-lstdc++fs)
    set(WIN32 ON)
endif()

#--------------------------------
# External libraries
#--------------------------------

add_subdirectory(lib/glm)

#--------------------------------
# Source
#--------------------------------

file(GLOB_RECURSE DECAY_LIBRARY_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)

add_library(DecayLib STATIC ${DECAY_LIBRARY_SOURCE})
target_include_directories(DecayLib PUBLIC src/)
add_library(Decay::Library ALIAS DecayLib)

target_link_libraries(DecayLib -static-libgcc -static-libstdc++ stdc++fs)
target_link_libraries(DecayLib glm)

target_include_directories(DecayLib PRIVATE lib/stb/)

#--------------------------------
# Source - Command
#--------------------------------

option(DECAY_LIBRARY_CMD "Enable Commandline interface/executable utility." ON)
if(DECAY_LIBRARY_CMD)
    file(GLOB_RECURSE DECAY_LIBRARY_CMD_SOURCE src_cmd/*.hpp src_cmd/*.cpp src_cmd/*.h src_cmd/*.c)

    add_executable(DecayLib_Command ${DECAY_LIBRARY_CMD_SOURCE})
    add_executable(Decay::LibraryCmd ALIAS DecayLib_Command)

    target_link_libraries(DecayLib_Command DecayLib)
    target_link_libraries(DecayLib_Command -static-libgcc -static-libstdc++ stdc++fs)
    target_link_libraries(DecayLib_Command glm)
endif()

#--------------------------------
# Tests
#--------------------------------

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
