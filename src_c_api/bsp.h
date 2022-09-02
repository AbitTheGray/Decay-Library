#pragma once

#include "wad.h"

#ifdef __cplusplus
#include "Decay/Bsp/v30/BspTree.hpp"

typedef Decay::Bsp::v30::BspFile bsp30_file;
typedef Decay::Bsp::v30::BspTree bsp30_tree;

typedef Decay::Bsp::v30::BspTree::Vertex bsp30_vertex;
typedef Decay::Bsp::v30::BspTree::Model bsp30_model;

typedef Decay::Bsp::v30::BspTree::Entity bsp30_entity;

typedef glm::vec3 bsp_vec3;
typedef glm::u8vec3 bsp_u8vec3;
#else
typedef struct {char dummy;} bsp30_file;
typedef struct {char dummy;} bsp30_tree;

typedef struct
{
    float x, y, z;
    float u, v;
    float light_u, light_v;
} bsp30_vertex;
typedef struct {char dummy;} bsp30_model;

typedef struct {char dummy;} bsp30_entity;

typedef struct
{
    float x, y, z;
} bsp_vec3;
typedef struct
{
    unsigned char x, y, z;
} bsp_u8vec3;
#endif

typedef struct
{
    bsp_vec3 min, max;
} bsp_bounding_box;

typedef struct
{
    const unsigned int width, height;
    const bsp_u8vec3* data;
} bsp_lightmap;

#ifdef __cplusplus
extern "C"
{
#endif

#define DECAY_ERR_ARG_NULL 0x01u
#define DECAY_ERR_ARG_OUT_OF_RANGE 0x02u

#define DECAY_ERR_FILE_NOT_FOUND 0x10u
#define DECAY_ERR_FILE_INVALID 0x11u
#define DECAY_ERR_FILE_PARSE_FAILED 0x12u

#define DECAY_ERR_BSP_TREE_CREATE_FAILED 0x20u

    /// Load BSP from filesystem
    /// Returns `NULL` if file was not found or there was problem with loading
    /// Returned pointer must be freed using `bsp_file_free(bsp_tree*)` (not required if `NULL`).
    bsp30_file* bsp_file_load(const char* path, char* error);
    /// Free BSP pointer from `bsp_file_load`
    void bsp_file_free(bsp30_file* bspFile);

    /// Constructs BSP Tree from BSP map
    /// Returns `NULL` when incorrect `bspFile` is supplied or there was problem with parsing it.
    /// Returned pointer must be freed using `bsp_tree_free(bsp_tree*)` (not required if `NULL`).
    bsp30_tree* bsp_tree_create(bsp30_file* bspFile, char* error);
    /// Free BSP Tree pointer from `bsp_tree_create`
    void bsp_tree_free(bsp30_tree* bspTree);

    /// Utility function to free `bsp_file*` and `bsp_tree*` using same function.
    static void bsp_free(bsp30_file* bspFile, bsp30_tree* bspTree)
    {
        bsp_file_free(bspFile);
        bsp_tree_free(bspTree);
    }

    /// Loads `bsp_tree*` directly from file.
    /// May return `NULL`.
    /// Returned pointer must be freed using `bsp_tree_free(bsp_tree*)` (not required if `NULL`).
    static bsp30_tree* bsp_tree_load(const char* path, char* error)
    {
        bsp30_file* bspFile = bsp_file_load(path, error);
        if(*error)
        {
#ifdef __cplusplus
            return nullptr;
#else
            return ((void*)0);
#endif
        }
        if(bspFile == (void*)0)
        {
#ifdef __cplusplus
            return nullptr;
#else
            return ((void*)0);
#endif
        }

        bsp30_tree* bspTree = bsp_tree_create(bspFile, error);

        bsp_file_free(bspFile);

        if(*error)
        {
#ifdef __cplusplus
            return nullptr;
#else
            return ((void*)0);
#endif
        }
        if(bspTree == (void*)0)
        {
#ifdef __cplusplus
            return nullptr;
#else
            return ((void*)0);
#endif
        }

        return bspTree; // May be ` nullptr`
    }

    /// Do not free `wad_rgba*` yourself
    /// Pointer lifetime is same as `bsp_tree*`
    bsp30_vertex* bsp_vertices(bsp30_tree* bspTree, int* length);

    /// Get all models from BSP Tree.
    /// Returns number of models.
    /// Caller has to allocate array of `bsp_model*` and pass it as `models` argument. Can be `NULL` to not write values.
    /// No freeing is required.
    /// `bsp_model*` lifetime is same as used `bsp_tree*`.
    int bsp_get_models(bsp30_tree* bspTree, bsp30_model** models);

    /// Get model by index from BSP Tree.
    /// `bsp_model*` lifetime is same as used `bsp_tree*`.
    bsp30_model* bsp_get_model(bsp30_tree* bspTree, int modelId);
    /// Get main model (static world) from BSP Tree .
    /// `bsp_model*` lifetime is same as used `bsp_tree*`.
    static bsp30_model* bsp_get_world_model(bsp30_tree* bspTree)
    {
        return bsp_get_model(bspTree, 0);
    }

    /// Get indices from model for specific texture index.
    /// Returns number of indices (for the texture index).
    /// Caller has to allocate array of `short*` and pass it as `indices` argument. Can be `NULL` to not write values.
    int bsp_model_get_indices(bsp30_model* model, int textureIndex, short* indices);

    /// Get textures used by specified `bsp_model`.
    /// Returns number of textures.
    /// If `textures` is not `NULL` then texture indexes are written into it. Caller has to allocate enough memory.
    int bsp_model_textures(bsp30_model* model, int* textures);

    /// Get origin (coordinates shift) of the model.
    bsp_vec3 bsp_model_origin(bsp30_model* model);

    /// Get Bounding-Box of the model.
    bsp_bounding_box bsp_model_bounding_box(bsp30_model* model);


    /// Load all textures inside BSP file
    /// Returns `nullptr` if there were no textures, file was not found or there was problem with loading
    wad_texture* bsp_tree_textures(bsp30_tree* bspTree, int* length);
    /// Release textures loaded from BSP file
    static void bsp_free_textures(wad_texture* textures)
    {
        wad_free_textures(textures);
    }


    /// Get entities from BSP Tree.
    /// Returns number of entities.
    /// If `entities` is not `NULL` then entities are written into it. Caller has to allocate enough memory.
    /// Lifetime of `bsp_entity*` is same as `bsp_tree*`.
    int bsp_tree_entities(const bsp30_tree* bspTree, const bsp30_entity** entities);

    /// Get entity's keys.
    /// Returns number of entities.
    /// If `keys` is not `NULL` then entities are written into it. Caller has to allocate enough memory.
    /// Lifetime of text values (strings) is same as `bsp_entity*` (same as `bsp_tree*`).
    int bsp_entity_keys(const bsp30_entity* entity, const char** keys);

    /// Get entity's value from `key`.
    /// Lifetime of text values (strings) is same as `bsp_entity*` (same as `bsp_tree*`).
    /// Returns `NULL` if the value was not found.
    const char* bsp_entity_value(const bsp30_entity* entity, const char* key);


    /// Get lightmap from BSP.
    /// Contains shadows and light to all faces which are not fully lit.
    bsp_lightmap bsp_light(const bsp30_tree* bspTree);

#ifdef __cplusplus
};
#endif
