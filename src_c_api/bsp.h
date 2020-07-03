#pragma once

#include "wad.h"

#ifdef __cplusplus
#include <Decay/Bsp/BspTree.hpp>

typedef Decay::Bsp::BspFile bsp_file;
typedef Decay::Bsp::BspTree bsp_tree;

typedef Decay::Bsp::BspTree::Vertex bsp_vertex;
typedef Decay::Bsp::BspTree::Model bsp_model;
#else
typedef struct {char dummy;} bsp_file;
typedef struct {char dummy;} bsp_tree;

typedef struct
{
    float x, y, z;
    float u, v;
    float s, t;
} bsp_vertex;
typedef struct {char dummy;} bsp_model;
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /// Load BSP from filesystem
    /// Returns `NULL` if file was not found or there was problem with loading
    /// Returned pointer must be freed using `bsp_file_free(bsp_tree*)` (not required if `NULL`).
    bsp_file* bsp_file_load(const char* path);
    /// Free BSP pointer from `bsp_file_load`
    void bsp_file_free(bsp_file* bspFile);

    /// Constructs BSP Tree from BSP map
    /// Returns `NULL` when incorrect `bspFile` is supplied or there was problem with parsing it.
    /// Returned pointer must be freed using `bsp_tree_free(bsp_tree*)` (not required if `NULL`).
    bsp_tree* bsp_tree_create(bsp_file* bspFile);
    /// Free BSP Tree pointer from `bsp_tree_create`
    void bsp_tree_free(bsp_tree* bspTree);

    /// Utility function to free `bsp_file*` and `bsp_tree*` using same function.
    static void bsp_free(bsp_file* bspFile, bsp_tree* bspTree)
    {
        bsp_file_free(bspFile);
        bsp_tree_free(bspTree);
    }

    /// Loads `bsp_tree*` directly from file.
    /// May return `NULL`.
    /// Returned pointer must be freed using `bsp_tree_free(bsp_tree*)` (not required if `NULL`).
    static bsp_tree* bsp_tree_load(const char* path)
    {
        bsp_file* bspFile = bsp_file_load(path);
        if(!bspFile)
        {
#ifdef __cplusplus
            return nullptr;
#else
            return ((void*)0);
#endif
        }

        bsp_tree* bspTree = bsp_tree_create(bspFile);

        bsp_file_free(bspFile);

        return bspTree; // May be ` nullptr`
    }

    /// Do not free `wad_rgba*` yourself
    /// Pointer lifetime is same as `bsp_tree*`
    bsp_vertex* bsp_vertices(bsp_tree* bspTree, int* length);

    /// Get all models from BSP Tree.
    /// Returns number of models.
    /// Caller has to allocate array of `bsp_model*` and pass it as `models` argument. Can be `NULL` to not write values.
    /// No freeing is required.
    /// `bsp_model*` lifetime is same as used `bsp_tree*`.
    int bsp_get_models(bsp_tree* bspTree, bsp_model** models);

    /// Get model by index from BSP Tree.
    /// `bsp_model*` lifetime is same as used `bsp_tree*`.
    bsp_model* bsp_get_model(bsp_tree* bspTree, int modelId);
    /// Get main model (static world) from BSP Tree .
    /// `bsp_model*` lifetime is same as used `bsp_tree*`.
    static bsp_model* bsp_get_world_model(bsp_tree* bspTree)
    {
        return bsp_get_model(bspTree, 0);
    }

    int bsp_model_get_indices(bsp_model* model, int textureIndex, short* indices);

    /// Get textures used by specified `bsp_model`.
    /// Returns number of textures.
    /// If `textures` is not `NULL` then texture indexes are written into it. Caller has to allocate enough memory.
    int bsp_model_textures(bsp_model* model, int* textures);


    /// Load all textures inside BSP file
    /// Returns `nullptr` if there were no textures, file was not found or there was problem with loading
    wad_texture* bsp_tree_textures(bsp_tree* bspTree, int* length);
    /// Release textures loaded from BSP file
    static void bsp_free_textures(wad_texture* textures)
    {
        wad_free_textures(textures);
    }

#ifdef __cplusplus
};
#endif
