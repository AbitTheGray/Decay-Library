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

# More lenient parsing of Input / Output in FGD entities - "()" means "(void)"
option(DECAY_FGD_IO_PARAM_VOID "Allow empty parameter brackets to mean `void`" ON)
if(DECAY_FGD_IO_PARAM_VOID)
    add_compile_definitions("FGD_IO_PARAM_VOID")
endif()

# By official documentation, only `flags` and `choices` support multiple values/choices.
# Turn this OFF to disable the check and allow it for more types of properties (in case you are creating a custom one?).
option(DECAY_FGD_PROPERTY_ITEMS_LIMIT_FLAGS_CHOICES "Items in properties are only available to `flags` and `choices` types" ON)
if(DECAY_FGD_PROPERTY_ITEMS_LIMIT_FLAGS_CHOICES)
    add_compile_definitions("FGD_PROPERTY_ITEMS_LIMIT_FLAGS_CHOICES")
endif()

# Checks otherwise unrecognized objects (other than map size, excluded game directories...) against known list of valid classes.
# Turn this off only when you need support for custom classes.
option(DECAY_FGD_CLASS_VALIDATE "Validate objects inside FGD against build-in list of valid classes" ON)
if(DECAY_FGD_CLASS_VALIDATE)
    add_compile_definitions("FGD_CLASS_VALIDATE")
endif()

# Link nlohmann/JSON library
option(DECAY_JSON_LIB "Use nlohmann's JSON library and allow JSON conversions" ON)
if(DECAY_JSON_LIB)
    add_compile_definitions("DECAY_JSON_LIB")
endif()

# Do not check length of values (and some other string) against official limits
option(DECAY_RMF_NO_LIMITS "No length limits in RMF file" OFF)
if(DECAY_RMF_NO_LIMITS)
    add_compile_definitions("RMF_NO_LIMITS")
endif()

# Will include "wad" in RMF which is otherwise omitted because it usually exceeds maximum value length
option(DECAY_RMF_INCLUDE_WAD "Include `wad` key-value in RMF (recommends DECAY_RMF_NO_LIMITS)" OFF)
if(DECAY_RMF_INCLUDE_WAD)
    add_compile_definitions("RMF_INCLUDE_WAD")
endif()

# Apply GoldSrc limit of classname for Path entities (separate list from Entities)
option(DECAY_RMF_PATH_TYPE_CHECK "Limit RMF's Path records only to `path_corner` and `path_track` entities" ON)
if(DECAY_RMF_PATH_TYPE_CHECK)
    add_compile_definitions("RMF_PATH_TYPE_CHECK")
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

add_subdirectory(lib)

#--------------------------------
# Source
#--------------------------------

file(GLOB_RECURSE DECAY_LIBRARY_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)

add_library(DecayLib STATIC ${DECAY_LIBRARY_SOURCE})
target_include_directories(DecayLib PUBLIC src/)
add_library(Decay::Library ALIAS DecayLib)

target_link_libraries(DecayLib -static-libgcc -static-libstdc++ stdc++fs)
target_link_libraries(DecayLib glm)

target_include_directories(DecayLib PUBLIC lib/stb/)

if(DECAY_LIBRARY_API)
    target_compile_options(DecayLib PUBLIC -fPIC)
endif()
if(DECAY_JSON_LIB)
    target_link_libraries(DecayLib nlohmann_json)
endif()

#--------------------------------
# Source - Command
#--------------------------------

if(DECAY_LIBRARY_CMD)
    file(GLOB_RECURSE DECAY_LIBRARY_CMD_SOURCE src_cmd/*.hpp src_cmd/*.cpp src_cmd/*.h src_cmd/*.c)

    add_executable(DecayLib_Command ${DECAY_LIBRARY_CMD_SOURCE})
    add_executable(Decay::LibraryCmd ALIAS DecayLib_Command)

    target_link_libraries(DecayLib_Command DecayLib)
    target_link_libraries(DecayLib_Command cxxopts)

    target_include_directories(DecayLib_Command PRIVATE lib/stb/)
endif()

#--------------------------------
# Tests
#--------------------------------

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
