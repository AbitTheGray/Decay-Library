#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    void* bsp_file_load(const char* path);
    void bsp_file_free(void* bsp_ptr);

    //TODO extract textures
    //TODO to Bsp Tree

#ifdef __cplusplus
};
#endif
