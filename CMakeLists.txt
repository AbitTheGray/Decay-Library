cmake_minimum_required(VERSION 3.16)
project(DecayLib CXX)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

cmake_policy(SET CMP0077 NEW) # set() overrides option()

#--------------------------------
# Debug Options
#--------------------------------

add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>") # Define DEBUG in Debug builds

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

file(GLOB_RECURSE DS1_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)

add_library(DecayLib STATIC ${DS1_SOURCE})
target_include_directories(DecayLib PUBLIC src/)
add_library(Decay::Library ALIAS DecayLib)

target_link_libraries(DecayLib -static-libgcc -static-libstdc++ stdc++fs)
target_link_libraries(DecayLib glm)

target_include_directories(DecayLib PRIVATE lib/stb/)

#--------------------------------
# Source
#--------------------------------

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
