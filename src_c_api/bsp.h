#pragma once

#ifdef __cplusplus
#include <Decay/Bsp/BspTree.hpp>

typedef Decay::Bsp::BspFile bsp_file;
typedef Decay::Bsp::BspTree bsp_tree;

typedef Decay::Bsp::BspTree::Vertex bsp_vertex;
#else
typedef struct {char dummy;} bsp_file;
typedef struct {char dummy;} bsp_tree;

typedef struct
{
    float x, y, z;
    float u, v;
    float s, t;
} bsp_vertex;
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /// Load BSP from filesystem
    /// Returns `NULL` if file was not found or there was problem with loading
    bsp_file* bsp_file_load(const char* path);
    /// Free BSP pointer from `bsp_file_load`
    void bsp_file_free(bsp_file* bspFile);

    /// Constructs BSP Tree from BSP map
    /// Returns `NULL` when incorrect `bspFile` is supplied or there was problem with parsing it.
    bsp_tree* bsp_tree_create(bsp_file* bspFile);
    /// Free BSP Tree pointer from `bsp_tree_create`
    void bsp_tree_free(bsp_tree* bspTree);

    void bsp_free(bsp_file* bspFile, bsp_tree* bspTree)
    {
        bsp_file_free(bspFile);
        bsp_tree_free(bspTree);
    }
    bsp_tree* bsp_tree_load(const char* path)
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
    bsp_vertex* bsp_vertices(bsp_tree* bspTree);


    //TODO extract textures from BSP
    //TODO get geometry from BSP Tree

#ifdef __cplusplus
};
#endif
