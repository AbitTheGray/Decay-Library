#include "wad.h"

#include "Decay/Wad/Wad3/WadFile.hpp"

#include "utility.hpp"

using namespace Decay::Wad::Wad3;

static wad_image* LoadImages(const std::map<std::string, WadFile::Image>& images, int* length)
{
    R_ASSERT(images.size() < std::numeric_limits<int>::max());
    *length = images.size();
    if(*length == 0)
        return nullptr;

    /// Size of allocated memory for `wadTextures` array
    std::size_t memorySize = *length * sizeof(wad_texture); // Instances of `wad_texture`
    for(auto & it : images)
        memorySize += sizeof(wad_rgba) * it.second.Data.size();

    wad_image* wadImages = static_cast<wad_image*>(malloc(memorySize));
    uint8_t* memoryEnd = reinterpret_cast<uint8_t*>(wadImages) + memorySize;

    /// Points to start of next data
    /// No need to clear it as it will always be used.
    wad_rgba* nextData = reinterpret_cast<wad_rgba*>(reinterpret_cast<uint8_t*>(wadImages + *length));

    std::size_t i = 0;
    for(auto& it : images)
    {
        auto& tex = it.second;
        auto& wt = wadImages[i];

        // Copy name
        R_ASSERT(it.first.size() < 16);
        CopyString(it.first, wt.name, 16);

        // Copy size
        wt.width = tex.Width;
        wt.height = tex.Height;

        // Invalid texture size
        if(tex.Width == 0 || tex.Height == 0)
        [[unlikely]]
        {
            wt.data = nullptr;
            continue;
        }

        // Texture does not contain RGBA data (only in BSP)
        if(tex.Data.empty())
        {
            wt.data = nullptr;
            continue;
        }

        // Load data
        try
        {
            std::size_t dataLength = tex.Width * tex.Height;

            // Convert texture to RGBA
            std::vector<glm::u8vec4> rgbaData = tex.AsRgba();
            if(dataLength != rgbaData.size())
            {
                wt.data = nullptr;
                continue;
            }

            // Copy RGBA data to `wad_texture`
            wt.data = nextData;
            for(std::size_t tdi = 0; tdi < dataLength; tdi++)
                wt.data[tdi] = rgbaData[tdi];
        }
        catch(std::exception& ex)
        {
            wt.data = nullptr;
        }

        nextData += tex.Data.size();
    }

    // Check for bleeding memory
    if(reinterpret_cast<uint8_t*>(nextData) != memoryEnd)
    [[unlikely]]
    {
        // Write error
        std::cerr << "wadTextures array ended " << (memoryEnd - reinterpret_cast<uint8_t*>(nextData)) << " bytes before end of allocated memory" << std::endl;
        // Fill memory by 0 to erase all bleeding data
        std::fill(reinterpret_cast<uint8_t*>(nextData), memoryEnd, 0);
    }

    return wadImages;
}

wad_image* wad_load_images(const char* path, int* length)
{
    *length = 0;

    if(path == nullptr || path[0] == '\0')
        return nullptr;

    std::filesystem::path filename(path);
    if(!std::filesystem::exists(filename))
        return nullptr; // File not found
    if(!std::filesystem::is_regular_file(filename))
        return nullptr; // Path target is not a file

    std::shared_ptr<WadFile> wad;
    try
    {
        wad = std::make_shared<WadFile>(filename);
    }
    catch(std::exception& ex)
    {
        return nullptr; // Failed to load WAD file
    }

    return LoadImages(wad->ReadAllImages_Map(), length);
}


void wad_free_images(wad_image* images)
{
    // Allocated as single big blob
    // Requires just this call
    free(images);
}
