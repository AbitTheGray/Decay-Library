#include <Decay/Wad/WadParser.hpp>
#include <iostream>

int main()
{
    using namespace Decay::Wad;

    for(const auto& dir : std::filesystem::recursive_directory_iterator("../../../half-life/"))
    {
        if(dir.is_directory())
            continue;
        if(dir.path().extension() != ".wad")
            continue;

        try
        {
            std::cout << dir.path() << std::endl;

            auto wadIt = WadParser(dir.path());

            std::size_t countTexture = 0;
            std::size_t countImage = 0;
            std::size_t countFont = 0;
            std::size_t countOther = 0;

            for(const auto& item : wadIt.GetItems())
            {
                switch(item.Type)
                {
                    case WadParser::ItemType::MipMapTexture:
                        countTexture++;
                        continue;
                    case WadParser::ItemType::Image:
                        countImage++;
                        continue;
                    case WadParser::ItemType::Font:
                        countFont++;
                        continue;
                    default:
                        countOther++;
                        break;
                }
                std::cout << item.Name << " = " << static_cast<uint32_t>(static_cast<uint8_t>(item.Type)) << std::endl;
            }

            std::cout << "Texture: " << countTexture <<
                       ", Image: " << countImage <<
                       ", Font: " << countFont <<
                       ", Other: " << countOther <<
            std::endl;
        }
        catch(std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }

}
