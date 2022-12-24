#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspTree.hpp"
#include "Decay/Common.hpp"

int main()
{
    using namespace Decay;
    using namespace Decay::Bsp::v30;

    auto bsp = std::make_shared<BspFile>("../../../half-life/cstrike/maps/de_dust2.bsp");

    auto tree = std::make_shared<BspTree>(bsp);

    std::function<void(const char* path, uint32_t width, uint32_t height, const glm::u8vec3* data)> writeFunc = ImageWriteFunction_RGB(std::string(".png"));
    writeFunc("light.png", tree->Light.Width, tree->Light.Height, tree->Light.Data.data());
}
