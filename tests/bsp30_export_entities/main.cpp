#include <iostream>

#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspTree.hpp"

int main()
{
    using namespace Decay::Bsp::v30;

    std::cout << "de_dust2.bsp:" << std::endl;
    auto bsp = std::make_shared<BspFile>("../../../half-life/cstrike/maps/de_dust2.bsp");

    void* data        = bsp->m_Data[(int)BspFile::LumpType::Entities];
    auto  data_Length = bsp->m_DataLength[(int)BspFile::LumpType::Entities];


    std::fstream("entities.bin", std::ios_base::out).write(static_cast<const char*>(data), data_Length);
}
