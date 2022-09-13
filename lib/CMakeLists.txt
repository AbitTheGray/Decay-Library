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
