set(BUILD_SHARED_LIBS OFF)
SET(BUILD_TESTING OFF) # No need to test libraries

#--------------------------------
# GLM
# Vertex and Matrix library
#--------------------------------

if(NOT TARGET glm)
    add_subdirectory(glm)
endif()

#--------------------------------
# nlohmann JSON
# JSON for Modern C++
#--------------------------------

if(NOT TARGET nlohmann_json)
    set(JSON_BuildTests OFF)
    set(JSON_Install OFF)
    add_subdirectory(nlohmann_json)
endif()



#--------------------------------
# cxxopts
# Command-line parsing library
#--------------------------------

if(NOT TARGET cxxopts)
    set(CXXOPTS_BUILD_EXAMPLES OFF)
    set(CXXOPTS_BUILD_TESTS OFF)
    set(CXXOPTS_ENABLE_INSTALL OFF)
    set(CXXOPTS_ENABLE_WARNINGS OFF)

    set(CXXOPTS_USE_UNICODE_HELP OFF)

    add_subdirectory(cxxopts)
endif()
