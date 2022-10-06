#include "Decay/Wad/Wad3/WadFile.hpp"

using namespace Decay::Wad::Wad3;

void Test_Image(const WadFile::Image& original)
{
    auto item = original.AsItem("test");
    R_ASSERT(!item.Data.empty(), "Image has no data");

    Decay::MemoryBuffer itemDataBuffer(reinterpret_cast<char*>(item.Data.data()), item.Data.size());
    std::istream in(&itemDataBuffer);

    WadFile::Image result(in);
    R_ASSERT(in.good() || in.eof(), "Stream is not in a good state after reading");

    R_ASSERT(result.Size == original.Size, "Image does not match");

    R_ASSERT(result.Data.size() == original.Data.size(), "Image data size does not match");
    for(int i = 0; i < result.Data.size(); i++)
        R_ASSERT(result.Data[i] == original.Data[i], "Pixel at index " << i << " does not match");

    R_ASSERT(result.Palette.size() == original.Palette.size(), "Image data size does not match");
    for(int i = 0; i < result.Palette.size(); i++)
        R_ASSERT(result.Palette[i] == original.Palette[i], "Pixel at index " << i << " does not match");

    //R_ASSERT(result == original, "Result does not match the original");
}

int main()
{
    // Image
    {
        std::cout << "Image" << std::endl;

        WadFile::Image image0(32, 64);
        for(int i = 0; i < image0.Data.size(); i++)
            image0.Data[i] = i % 256;
        image0.Palette = WadFile::Image::RainbowPalette;

        Test_Image(image0);

        std::cout << std::endl;
    }
}
