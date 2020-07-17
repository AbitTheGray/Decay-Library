cmake_minimum_required(VERSION 3.16)
project(DecayLib C CXX)

set(CMAKE_CXX_STANDARD 20)

cmake_policy(SET CMP0077 NEW) # set() overrides option()

#--------------------------------
# Options
#--------------------------------

add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>") # Define DEBUG in Debug builds


# Enable C API
# Builds another target as Shared library
option(DECAY_LIBRARY_API "Enable C API build." ON)

# Enable Command-line utility target
option(DECAY_LIBRARY_CMD "Enable Commandline interface/executable utility." ON)


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

# Use ST (texture coordinates) instead of UV (normalized coordinates) for texture mapping on faces
# WARNING: Exporting OBJ will use ST texture coordinates instead of UV
option(DECAY_BSP_ST_INSTEAD_OF_UV "Use ST (0 to size) texture coordinates instead of UV (0 to 1)." OFF)
if(DECAY_BSP_ST_INSTEAD_OF_UV)
    add_compile_definitions("DECAY_BSP_ST_INSTEAD_OF_UV")
endif()

# Use ST (texture coordinates) instead of UV (normalized coordinates) for LIGHTMAP texture mapping on faces
option(DECAY_BSP_LIGHTMAP_ST_INSTEAD_OF_UV "Use ST (0 to size) lightmap texture coordinates instead of UV (0 to 1)." OFF)
if(DECAY_BSP_LIGHTMAP_ST_INSTEAD_OF_UV)
    add_compile_definitions("DECAY_BSP_LIGHTMAP_ST_INSTEAD_OF_UV")
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

if(DECAY_LIBRARY_API)
    target_compile_options(DecayLib PUBLIC -fPIC)
endif()

#--------------------------------
# Source - C API
#--------------------------------

if(DECAY_LIBRARY_API)
    file(GLOB_RECURSE DECAY_LIBRARY_API_SOURCE src_c_api/*.hpp src_c_api/*.cpp src_c_api/*.h src_c_api/*.c)

    add_library(DecayLib_API SHARED ${DECAY_LIBRARY_API_SOURCE})
    target_include_directories(DecayLib_API PUBLIC src_c_api/)
    add_library(Decay::LibraryAPI ALIAS DecayLib_API)

    target_link_libraries(DecayLib_API DecayLib)
    target_link_libraries(DecayLib_API -static-libgcc -static-libstdc++ stdc++fs)
    target_link_libraries(DecayLib_API glm)
endif()

#--------------------------------
# Source - Command
#--------------------------------

if(DECAY_LIBRARY_CMD)
    file(GLOB_RECURSE DECAY_LIBRARY_CMD_SOURCE src_cmd/*.hpp src_cmd/*.cpp src_cmd/*.h src_cmd/*.c)

    add_executable(DecayLib_Command ${DECAY_LIBRARY_CMD_SOURCE})
    add_executable(Decay::LibraryCmd ALIAS DecayLib_Command)

    target_link_libraries(DecayLib_Command DecayLib)
    target_link_libraries(DecayLib_Command -static-libgcc -static-libstdc++ stdc++fs)
    target_link_libraries(DecayLib_Command glm)

    target_include_directories(DecayLib_Command PRIVATE lib/stb/)
endif()

#--------------------------------
# Tests
#--------------------------------

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
