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

    // OBJ = 3D model
    {
        std::fstream out = std::fstream(std::filesystem::path(argv[1]).filename().string() + std::string(".obj"), std::ios_base::out);
        if(!mdl.BodyParts.empty())
        {
            int vertexIndex = 0;

            out << "# Generated using Decay Library" << std::endl;
            out << "# From " << argv[1] << std::endl;
            out << "mtllib " << (std::filesystem::path(argv[1]).filename().string() + std::string(".mtl")) << std::endl;

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

                        const auto& texture = mdl.Textures[kv.first];
                        out << "usemtl " << texture.first << std::endl;

                        for(int i = 0; i < kv.second.size(); i += 3, vertexIndex += 3)
                        {
                            out << "v " << kv.second[i + 0].Vertex.x << " " << kv.second[i + 0].Vertex.y << " " << kv.second[i + 0].Vertex.z << std::endl;
                            out << "v " << kv.second[i + 1].Vertex.x << " " << kv.second[i + 1].Vertex.y << " " << kv.second[i + 1].Vertex.z << std::endl;
                            out << "v " << kv.second[i + 2].Vertex.x << " " << kv.second[i + 2].Vertex.y << " " << kv.second[i + 2].Vertex.z << std::endl;

                            out << "vt " << (kv.second[i + 0].S / (float)texture.second.Width) << " " << (kv.second[i + 0].T / (float)texture.second.Height) << std::endl;
                            out << "vt " << (kv.second[i + 1].S / (float)texture.second.Width) << " " << (kv.second[i + 1].T / (float)texture.second.Height) << std::endl;
                            out << "vt " << (kv.second[i + 2].S / (float)texture.second.Width) << " " << (kv.second[i + 2].T / (float)texture.second.Height) << std::endl;

                            out << "vn " << kv.second[i + 0].Normal.x << " " << kv.second[i + 0].Normal.y << " " << kv.second[i + 0].Normal.z << std::endl;
                            out << "vn " << kv.second[i + 1].Normal.x << " " << kv.second[i + 1].Normal.y << " " << kv.second[i + 1].Normal.z << std::endl;
                            out << "vn " << kv.second[i + 2].Normal.x << " " << kv.second[i + 2].Normal.y << " " << kv.second[i + 2].Normal.z << std::endl;

                            out << "f " << (vertexIndex + 1) << '/' << (vertexIndex + 1) << '/' << (vertexIndex + 1)
                                << ' ' << (vertexIndex + 2) << '/' << (vertexIndex + 2) << '/' << (vertexIndex + 2)
                                << " " << (vertexIndex + 3) << '/' << (vertexIndex + 3) << '/' << (vertexIndex + 3) << std::endl;
                        }
                    }
                }
            }
        }
        else
            std::cerr << "No body parts" << std::endl;
    }

    // MTL = Material
    {
        std::fstream out = std::fstream(std::filesystem::path(argv[1]).filename().string() + std::string(".mtl"), std::ios_base::out);
        if(!mdl.Textures.empty())
        {
            for(const auto& kv : mdl.Textures)
            {
                out << "newmtl " << kv.first << std::endl;
                out << "map_Ka " << kv.first << std::endl;
                out << "map_Kd " << kv.first << std::endl;
                out << "illum 2" << std::endl;

                out << std::endl;

                kv.second.WriteRgbPng(kv.first);
            }
        }
        else
            std::cout << "No textures" << std::endl;
    }
}
