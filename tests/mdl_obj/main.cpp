#include "Decay/Mdl/MdlFile.hpp"

#include "glm/gtx/string_cast.hpp"

using namespace Decay::Mdl;

int main(int argc, const char* argv[])
{
    if(argc < 2)
        throw std::runtime_error("Please provide path to MDL file");
#ifdef DEBUG
    std::cout << argv[1] << std::endl;
#endif

    std::fstream in = std::fstream(argv[1], std::ios_base::in | std::ios_base::binary);
    MdlFile mdl(in);

    std::fstream out = std::fstream(std::filesystem::path(argv[1]).filename().string() + std::string(".obj"), std::ios_base::out);
    if(!mdl.BodyParts.empty())
    {
        int vertexIndex = 0;

        out << "# Generated using Decay Library" << std::endl;
        out << "# From " << argv[1] << std::endl;

        for(const auto& bodypart : mdl.BodyParts)
        {
            for(const auto& model : bodypart.Models)
            {
                out << std::endl;
                out << "o " << bodypart.Name << " - " << model.Name << std::endl;
                for(const auto& kv : model.Meshes)
                {
                    R_ASSERT(kv.first < mdl.Textures.size(), "Texture index out of bounds");
                    R_ASSERT(kv.second.size() % 3 == 0, "Vertices do not form triangles");
                    
                    out << "usemtl " << mdl.Textures[kv.first].first << std::endl;
                    
                    for(int i = 0; i < kv.second.size(); i += 3, vertexIndex += 3)
                    {
                        out << "v " << kv.second[i + 0].Vertex.x << " " << kv.second[i + 0].Vertex.y << " " << kv.second[i + 0].Vertex.z << std::endl;
                        out << "v " << kv.second[i + 1].Vertex.x << " " << kv.second[i + 1].Vertex.y << " " << kv.second[i + 1].Vertex.z << std::endl;
                        out << "v " << kv.second[i + 2].Vertex.x << " " << kv.second[i + 2].Vertex.y << " " << kv.second[i + 2].Vertex.z << std::endl;
                        
                        out << "f " << (vertexIndex + 1) << " " << (vertexIndex + 2) << " " << (vertexIndex + 3) << std::endl;
                    }
                }
            }
        }
    }
    else
        std::cerr << "No body parts" << std::endl;
}
