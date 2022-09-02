#include "bsp.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
    #error This is C file, not C++
#endif

int main(int argc, char* argv[])
{
    char err = 0;
    bsp30_tree* bsp = bsp_tree_load("../../../half-life/cstrike/maps/de_dust2.bsp", &err);

    // Vertices
    {
        int verticesCount = 0;
        bsp30_vertex* vertices = bsp_vertices(bsp, &verticesCount);
    }


    // Textures (bsp)
    int texturesCount = 0;
    wad_texture* textures = bsp_tree_textures(bsp, &texturesCount);

    for(int textureIndex = 0; textureIndex < texturesCount; textureIndex++)
    {
        wad_texture texture = textures[textureIndex];

        if(texture.data == NULL)
            printf("%s (%ux%u)\n", texture.name, texture.width, texture.height);
        else
            printf("%s (%ux%u) + data\n", texture.name, texture.width, texture.height);
    }
    printf("\n");

    // Models
    {
        int modelCount = bsp_get_models(bsp, NULL);
        bsp30_model** models = malloc(sizeof(bsp30_model*) * modelCount);

        if(modelCount != bsp_get_models(bsp, models))
            return 1;

        // Loop models
        for(int modelIndex = 0; modelIndex < modelCount; modelIndex++)
        {
            bsp30_model* model = models[modelIndex];

            int usedTextureCount = bsp_model_textures(model, NULL);
            int* usedTextures = malloc(sizeof(int) * usedTextureCount);

            if(usedTextureCount != bsp_model_textures(model, usedTextures))
                return 1;

            // Loop textures
            for(int uti = 0; uti < usedTextureCount; uti++)
            {
                int textureIndex = usedTextures[uti];

                int indicesCount = bsp_model_get_indices(model, textureIndex, NULL);
                short* indices = malloc(sizeof(short) * indicesCount);
                if(indicesCount != bsp_model_get_indices(model, textureIndex, indices))
                    return 1;
            }
        }
    }

    // Entities
    {
        int entityCount = bsp_tree_entities(bsp, NULL);
        const bsp30_entity** entities = malloc(sizeof(bsp30_entity*) * entityCount);

        if(entityCount == 0)
        {
            printf("No entities");
        }
        else
        {
            if(entityCount != bsp_tree_entities(bsp, entities))
                return 0;

            printf("classname of 1st entity: %s\n", bsp_entity_value(entities[0], "classname"));
        }
    }

    bsp_tree_free(bsp);
}
