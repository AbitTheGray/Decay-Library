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
            std::cout << dir.path() <<std::endl;

            auto wadIt = WadParser(dir.path());

            for(const auto& item : wadIt.GetItems())
            {
                switch(item.Type)
                {
                    case WadParser::ItemType::MipMapTexture:
                    case WadParser::ItemType::Image:
                    case WadParser::ItemType::Font:
                        continue;
                    default:
                        break;
                }
                std::cout << item.Name << " = " << static_cast<uint32_t>(static_cast<uint8_t>(item.Type)) << std::endl;
            }
        }
        catch(std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }

}
