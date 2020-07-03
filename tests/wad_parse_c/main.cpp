#include <wad.h>
#include <iostream>

int main()
{
    int textures_count = 0;
    wad_texture* textures = wad_load_textures("../../../half-life/valve/halflife.wad", &textures_count);

    std::cout << "Textures: " << textures_count << std::endl;

    for(int i = 0; i < textures_count; i++)
    {
        wad_texture& t = textures[i];
        std::cout << t.name << " (" << t.width << "x" << t.height << ")" << std::endl;
    }

    wad_free_textures(textures);
}
