#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    std::cout << "halflife.wad:" << std::endl;
    std::fstream in("../../../half-life/valve/halflife.wad", std::ios_base::in | std::ios_base::binary);
    R_ASSERT(in.good(), "Failed to open the file");
    auto wad = WadFile(in);

    std::cout << "- Total Items: " << wad.GetItems().size() << std::endl;
    std::cout << "  - Images: " << wad.GetImageCount() << std::endl;
    std::cout << "  - Textures: " << wad.GetTextureCount() << std::endl;
    std::cout << "  - Fonts: " << wad.GetFontCount() << std::endl;
    std::cout << "  - Other: " << (wad.GetItems().size() - wad.GetImageCount() - wad.GetTextureCount() - wad.GetFontCount()) << std::endl;
}
