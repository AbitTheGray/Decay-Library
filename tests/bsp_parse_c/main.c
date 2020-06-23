#include <bsp.h>

#ifdef __cplusplus
    #error This is C file, not C++
#endif

int main(int argc, char* argv[])
{
    bsp_tree* bsp = bsp_tree_load("../../../half-life/cstrike/maps/de_dust2.bsp");

    bsp_tree_free(bsp);
}
