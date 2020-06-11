#include <Decay/Bsp/BspParser.hpp>
#include <iostream>

int main()
{
    using namespace Decay::Bsp;

    std::cout << "de_dust2.bsp:" << std::endl;
    auto bsp = BspParser("../../../half-life/cstrike/maps/de_dust2.bsp");

    std::cout << "- Vertices: " << bsp.GetVertexCount() << std::endl;
    std::cout << "- Faces: " << bsp.GetFaceCount() << std::endl;
    std::cout << "- Edges: " << bsp.GetEdgeCount() << std::endl;


    std::cout << std::endl;


    auto tree = bsp.AsNodeTree();

    std::cout << "Tree:" << std::endl;
    std::cout << "- Vertices: " << tree->Vertices.size() << std::endl;
    std::cout << "- Textures: " << tree->Textures.size() << std::endl;
    std::cout << "- Main Node: " << std::endl;
    std::cout << "  - Textures used: " << tree->MainNode->Indices.size() << " (top level)" << std::endl;
}
