#include <iostream>

#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspTree.hpp"

int main()
{
    using namespace Decay::Bsp::v30;

    std::cout << "de_dust2.bsp:" << std::endl;
    auto bsp = std::make_shared<BspFile>("../../../half-life/cstrike/maps/de_dust2.bsp");

    std::cout << "- Vertices: " << bsp->GetVertexCount() << std::endl;
    std::cout << "- Faces: " << bsp->GetFaceCount() << std::endl;
    std::cout << "- Edges: " << bsp->GetEdgeCount() << std::endl;


    std::cout << std::endl;


    auto tree = BspTree(bsp);

    std::cout << "Tree:" << std::endl;
    std::cout << "- Vertices: " << tree.Vertices.size() << std::endl;
    std::cout << "- Textures: " << tree.Textures.size() << std::endl;

    auto mainModel = tree.Models[0];
    std::cout << "- Main Model: " << std::endl;
    std::cout << "  - Textures used: " << mainModel->Indices.size() << " (top level)" << std::endl;
}
