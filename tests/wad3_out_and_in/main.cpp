#include "Decay/Wad/Wad3/WadFile.hpp"

using namespace Decay::Wad::Wad3;

void Test_Image(const WadFile::Image& original)
{
    std::stringstream ss;
    ss << original;
    R_ASSERT(ss.good(), "Stream is not in a good state");

#ifdef DEBUG
    std::cout << ss.str() << std::endl;
#endif
    ss.seekg(0, std::ios_base::beg);
    ss.seekp(0, std::ios_base::beg);

    MapFile::Face result = {};
    ss >> result;
    R_ASSERT(ss.good() || ss.eof(), "Stream is not in a good state after reading");

    R_ASSERT(result.Texture == original.Texture, "Face texture does not match");
    R_ASSERT(result == original, "Result does not match the original");
}

int main()
{
    // Image
    {
        std::cout << "Image" << std::endl;

        WadFile::Image image0 = {
            //TODO
        };

        Test_Image(image0);

        std::cout << std::endl;
    }
}
