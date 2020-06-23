#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct {char dummy;} bsp_file;
    typedef struct {char dummy;} bsp_tree;

    /// Load BSP from filesystem
    /// Returns `nullptr` if file was not found or there was problem with loading
    bsp_file* bsp_file_load(const char* path);
    /// Free BSP pointer from `bsp_file_load`
    void bsp_file_free(bsp_file* bspFile);

    /// Constructs BSP Tree from BSP map
    /// Returns `nullptr` when incorrect `bspFile` is supplied or there was problem with parsing it.
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
        if(bspFile == nullptr)
            return nullptr;

        bsp_tree* bspTree = bsp_tree_create(bspFile);

        bsp_file_free(bspFile);

        return bspTree; // May be ` nullptr`
    }


    //TODO extract textures from BSP
    //TODO get geometry from BSP Tree

#ifdef __cplusplus
};
#endif
