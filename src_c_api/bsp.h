#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    typedef void bsp_file;
    typedef void bsp_tree;

    /// Load BSP from filesystem
    bsp_file* bsp_file_load(const char* path);
    /// Free BSP pointer from `bsp_file_load`
    void bsp_file_free(bsp_file* bspFile);

    /// Constructs BSP Tree from BSP map
    /// Returns `nullptr` when incorrect `bspFile` is supplied.
    bsp_tree* bsp_tree_create(bsp_file* bspFile);
    /// Free BSP Tree pointer from `bsp_tree_create`
    void bsp_tree_free(bsp_tree* bspTree);

    //TODO extract textures from BSP
    //TODO get geometry from BSP Tree

#ifdef __cplusplus
};
#endif
