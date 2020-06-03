#include <Decay/Bsp/BspParser.hpp>
#include <iostream>

int main()
{
    auto bsp = Decay::Bsp::BspParser("../../../half-life/cstrike/maps/de_dust2.bsp");

    std::cout << "Vertices: " << bsp.GetVertexCount() << std::endl;
    std::cout << "Faces: " << bsp.GetFaceCount() << std::endl;
    std::cout << "Edges: " << bsp.GetEdgeCount() << std::endl;
}
