#include "Decay/Wad/Wad3/WadFile.hpp"
#include <iostream>

int main()
{
    using namespace Decay::Wad::Wad3;

    for(const auto& dir : std::filesystem::recursive_directory_iterator("../../../half-life/"))
    {
        if(dir.is_directory())
            continue;
        if(dir.path().extension() != ".wad")
            continue;

        try
        {
            std::cout << dir.path() << std::endl;

            std::fstream in(dir.path(), std::ios_base::in | std::ios_base::binary);
            R_ASSERT(in.good(), "Failed to open the file");
            auto wadIt = WadFile(in);

            std::size_t countTexture = 0;
            std::size_t countImage = 0;
            std::size_t countFont = 0;
            std::size_t countOther = 0;

            for(const auto& item : wadIt.GetItems())
            {
                switch(item.Type)
                {
                    case WadFile::ItemType::Texture:
                        countTexture++;
                        continue;
                    case WadFile::ItemType::Image:
                        countImage++;
                        continue;
                    case WadFile::ItemType::Font:
                        countFont++;
                        continue;
                    default:
                        countOther++;
                        break;
                }

                if(item.Type != static_cast<WadFile::ItemType>(0))
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
