#include "wad.h"

#include <Decay/Wad/WadFile.hpp>

using namespace Decay::Wad;

static void CopyString(const std::string& str, char* dest, int maxLength)
{
    if(str.size() > maxLength)
    {
        std::fill(dest, dest + maxLength, '\0');
        return;
    }

    // Copy `str`
    std::copy(str.c_str(), str.c_str() + str.length(), dest);

    // Fill rest by '\0'
    if(str.length() < maxLength)
        std::fill(dest + str.length(), dest + maxLength, '\0');
}

wad_texture** wad_load_textures(const char* path, int* length)
{
    std::shared_ptr<WadFile> wad = std::make_shared<WadFile>(path);

    auto textures = wad->ReadAllTextures();

    assert(textures.size() < std::numeric_limits<int>::max());
    *length = textures.size();
    wad_texture** wadTextures = static_cast<wad_texture**>(malloc(*length * sizeof(wad_texture*)));

    for(std::size_t i = 0; i < textures.size(); i++)
    {
        auto& tex = textures[i];

        assert(tex.Name.size() < 16);

        if(tex.Width == 0 || tex.Height == 0)
        {
            wadTextures[i] = static_cast<wad_texture*>(malloc(sizeof(wad_texture)));
            wadTextures[i]->width = 0;
            wadTextures[i]->height = 0;

            CopyString(tex.Name, wadTextures[i]->name, 16);

            wadTextures[i]->data = wad_rgba{ 0, 0, 0, 0 };
        }
        else
        {
            wadTextures[i] = static_cast<wad_texture*>(malloc(sizeof(uint32_t) * 2 + sizeof(wad_rgba) * tex.Width * tex.Height));
            wadTextures[i]->width = tex.Width;
            wadTextures[i]->height = tex.Height;

            CopyString(tex.Name, wadTextures[i]->name, 16);

            assert(tex.Width * tex.Height == tex.MipMapData[0].size());

            auto rgbaData = tex.AsRgba();
            assert(tex.Width * tex.Height == rgbaData.size());

            static_assert(sizeof(glm::u8vec4) == sizeof(wad_rgba));
            std::copy(rgbaData.begin(), rgbaData.end(), reinterpret_cast<glm::u8vec4*>(&wadTextures[i]->data));
        }
    }

    return wadTextures;
}

void wad_free_texture(wad_texture* texture)
{
    free(texture);
}

void wad_free_textures(int length, wad_texture** textures)
{
    for(; length >= 0; length--)
        wad_free_texture(textures[length]);

    free(textures);
}
