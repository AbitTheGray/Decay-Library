#include <Decay/Wad/WadParser.hpp>
#include <iostream>

int main()
{
    using namespace Decay::Wad;

    std::cout << "halflife.wad:" << std::endl;
    auto wad = WadParser("../../../half-life/valve/halflife.wad");

    std::cout << "- Total Items: " << wad.GetItems().size() << std::endl;
    std::cout << "  - Images: " << wad.GetImageCount() << std::endl;
    std::cout << "  - Textures: " << wad.GetTextureCount() << std::endl;
    std::cout << "  - Fonts: " << wad.GetFontCount() << std::endl;
    std::cout << "  - Other: " << (wad.GetItems().size() - wad.GetImageCount() - wad.GetTextureCount() - wad.GetFontCount()) << std::endl;
}
