#include "MdlFile.hpp"

#include "Decay/CommonReadUtils.hpp"

// Parsing utils
namespace Decay::Mdl
{
    MdlFile::MdlFile(std::istream& in)
    {
        in >> *this;
        if(in.fail())
            throw std::runtime_error("Failed to read Rmf file");
    }
}

// Stream Operators
namespace Decay::Mdl
{
    std::istream& operator>>(std::istream& in, MdlFile::Raw_BasicInfo& visGroup)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        in.read(reinterpret_cast<char*>(&visGroup), sizeof(MdlFile::Raw_BasicInfo));

        R_ASSERT(IsNullTerminated(visGroup.Name, MdlFile::Raw_BasicInfo::Name_Length), "VisGroup name must end with NULL character");

        //throw std::runtime_error("Not Implemented"); //TODO

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const MdlFile::Raw_BasicInfo& visGroup)
    {
        out.write(reinterpret_cast<const char*>(&visGroup), sizeof(MdlFile::Raw_BasicInfo));

        return out;
    }

    std::istream& operator>>(std::istream& in, MdlFile& mdl)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        { // Magic
            char magic[4];
            in.read(magic, sizeof(magic));

            //TODO check

#ifdef DEBUG
            std::cout << magic[0] << magic[1] << magic[2] << magic[3] << std::endl;
#endif
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        { // Version
            int version{};
            in.read(reinterpret_cast<char*>(&version), sizeof(version));

            R_ASSERT(version == 10, "Invalid file version");
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        MdlFile::Raw_BasicInfo basicInfo{};
        in >> basicInfo;

        {
            mdl.Info.Name = basicInfo.Name_str();
            mdl.Info.Size = basicInfo.Size;
            mdl.Info.EyePosition = basicInfo.EyePosition;
            mdl.Info.Min = basicInfo.Min;
            mdl.Info.Max = basicInfo.Max;
            mdl.Info.BBMin = basicInfo.BBMin;
            mdl.Info.BBMax = basicInfo.BBMax;
            mdl.Info.Flags = basicInfo.Flags;
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.BoneCount)
        {
            mdl.Bones.resize(basicInfo.BoneCount);
            in.seekg(basicInfo.BoneIndex, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.Bones.data()), sizeof(MdlFile::Bone) * mdl.Bones.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.BoneControllerCount)
        {
            mdl.BoneControllers.resize(basicInfo.BoneControllerCount);
            in.seekg(basicInfo.BoneControllerIndex, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.BoneControllers.data()), sizeof(MdlFile::BoneController) * mdl.BoneControllers.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.HitboxCount)
        {
            mdl.Hitboxes.resize(basicInfo.HitboxCount);
            in.seekg(basicInfo.HitboxIndex, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.Hitboxes.data()), sizeof(MdlFile::BBox) * mdl.Hitboxes.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.SeqCount)
        {
            mdl.SeqDescriptions.resize(basicInfo.SeqCount);
            in.seekg(basicInfo.SeqIndex, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.SeqDescriptions.data()), sizeof(MdlFile::SeqDescription) * mdl.SeqDescriptions.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.SeqGroupCount)
        {
            mdl.SeqGroups.resize(basicInfo.SeqGroupCount);
            in.seekg(basicInfo.SeqGroupIndex, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.SeqGroups.data()), sizeof(MdlFile::SeqGroup) * mdl.SeqGroups.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.TextureCount)
        {
            std::vector<MdlFile::Raw_Texture> textures(basicInfo.TextureCount);
            in.seekg(basicInfo.TextureIndex, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(textures.data()), sizeof(MdlFile::Raw_Texture) * textures.size());

            mdl.Textures.reserve(textures.size());

            for(const auto& texture : textures)
            {
                in.seekg(texture.Index, std::ios_base::beg);

                auto t = MdlFile::Texture((uint32_t)texture.Width, (uint32_t)texture.Height);

                R_ASSERT(in.good(), "Input stream is not in a good shape");
                t.Data = std::vector<uint8_t>(texture.Width * texture.Height);
                in.read(reinterpret_cast<char*>(t.Data.data()), t.Data.size());

                R_ASSERT(in.good(), "Input stream is not in a good shape");
                std::array<glm::u8vec3, 256> palette{};
                in.read(reinterpret_cast<char*>(palette.data()), sizeof(glm::u8vec3) * palette.size());
                t.Palette.resize(palette.size());
                for(int i = 0; i < palette.size(); i++)
                    t.Palette[i] = palette[i];

                mdl.Textures.emplace_back(texture.Name_str(), t);
            }
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        //TODO Skin
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.BodypartCount)
        {
            std::vector<MdlFile::Raw_BodyPart> bodyparts{};
            bodyparts.resize(basicInfo.BodypartCount);
            in.seekg(basicInfo.BodypartIndex, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(bodyparts.data()), sizeof(MdlFile::Raw_BodyPart) * bodyparts.size());

            mdl.BodyParts.reserve(bodyparts.size());
            for(const auto& bodypart : bodyparts)
            {
                MdlFile::BodyPart outBodyPart{};
                {
                    outBodyPart.Name = bodypart.Name_str();
                    outBodyPart.Base = bodypart.Base;

                    // Models
                    {
                        std::vector<MdlFile::Raw_Model> models(bodypart.ModelCount);
                        in.seekg(bodypart.ModelIndex, std::ios_base::beg);
                        in.read(reinterpret_cast<char*>(models.data()), sizeof(MdlFile::Raw_Model) * models.size());
                        R_ASSERT(in.good(), "Input stream is not in a good shape");

                        outBodyPart.Models.reserve(models.size());
                        for(const auto& model : models)
                        {
                            //std::cout << "Model: " << model.Name_str() << std::endl;

                            MdlFile::Model outModel{};
                            outModel.Name = model.Name_str();
                            outModel.Type = model.Type;
                            outModel.BoundingRadius = model.BoundingRadius;
                            {
                                // Vertices
                                std::vector<glm::vec3> vertices{};
                                {
                                    vertices.resize(model.VertexCount);
                                    in.seekg(model.VertexIndex, std::ios_base::beg);
                                    in.read(reinterpret_cast<char*>(vertices.data()), sizeof(glm::vec3) * vertices.size());
                                    R_ASSERT(in.good(), "Input stream is not in a good shape");
                                }
                                // Vertex Groups / Bone map
                                std::vector<uint8_t> verticeGroups{};
                                {
                                    verticeGroups.resize(model.VertexCount);
                                    in.seekg(model.VertexInfoIndex, std::ios_base::beg);
                                    in.read(reinterpret_cast<char*>(verticeGroups.data()), sizeof(uint8_t) * verticeGroups.size());
                                    R_ASSERT(in.good(), "Input stream is not in a good shape");
                                }

                                // Normals
                                std::vector<glm::vec3> normals{};
                                {
                                    normals.resize(model.NormalCount);
                                    in.seekg(model.NormalIndex, std::ios_base::beg);
                                    in.read(reinterpret_cast<char*>(normals.data()), sizeof(glm::vec3) * normals.size());
                                    R_ASSERT(in.good(), "Input stream is not in a good shape");
                                }
                                // Normal Groups
                                std::vector<uint8_t> normalGroups{};
                                {
                                    normalGroups.resize(model.NormalCount);
                                    in.seekg(model.NormalInfoIndex, std::ios_base::beg);
                                    in.read(reinterpret_cast<char*>(normalGroups.data()), sizeof(uint8_t) * normalGroups.size());
                                    R_ASSERT(in.good(), "Input stream is not in a good shape");
                                }

                                // Mesh
                                {
                                    std::vector<MdlFile::Raw_Mesh> meshes(model.MeshCount);
                                    in.seekg(model.MeshIndex, std::ios_base::beg);
                                    in.read(reinterpret_cast<char*>(meshes.data()), sizeof(MdlFile::Raw_Mesh) * meshes.size());
                                    R_ASSERT(in.good(), "Input stream is not in a good shape");

                                    for(const auto& mesh : meshes)
                                    {
                                        in.seekg(mesh.TriangleIndex, std::ios_base::beg);
                                        for(int ti = 0; ti < mesh.TriangleCount; ti++) // Triangle Fans
                                        {
                                            int16_t count;
                                            in.read(reinterpret_cast<char*>(&count), sizeof(count));
                                            R_ASSERT(in.good(), "Input stream is not in a good shape");

                                            if(count == 0)
                                                break; // End before finish

                                            int count_abs = std::abs(count);
                                            std::vector<MdlFile::Raw_Vertex> triangleVertices(count_abs);
                                            in.read(reinterpret_cast<char*>(triangleVertices.data()), sizeof(MdlFile::Raw_Vertex) * triangleVertices.size());
                                            R_ASSERT(in.good(), "Input stream is not in a good shape");

                                            R_ASSERT(count_abs >= 3, "Triangle fan must have at least 3 vertices");
                                            std::vector<MdlFile::Vertex>& meshVertices = outModel.Meshes[mesh.TextureID];
                                            if(count > 0) // Triangle Strip
                                            {
                                                //std::cout << "# " << ti << " Triangle Strip (" << count_abs << ")" << std::endl;

                                                meshVertices.reserve((count_abs - 2) * 3);

                                                for(int vi = 2; vi < count_abs; vi++)
                                                {
                                                    const auto& vi0 = triangleVertices[vi - 2];
                                                    R_ASSERT(vi0.VertexIndex < vertices.size(), "Vertex of triangle fan is out of bounds");
                                                    R_ASSERT(vi0.NormalIndex < normals.size(), "Normal of triangle fan is out of bounds");
                                                    meshVertices.emplace_back(
                                                        vertices[vi0.VertexIndex],
                                                        normals[vi0.NormalIndex],
                                                        vi0.S,
                                                        vi0.T
                                                    );

                                                    const auto& vi1 = triangleVertices[vi - 1];
                                                    R_ASSERT(vi1.VertexIndex < vertices.size(), "Vertex of triangle fan is out of bounds");
                                                    R_ASSERT(vi1.NormalIndex < normals.size(), "Normal of triangle fan is out of bounds");
                                                    meshVertices.emplace_back(
                                                        vertices[vi1.VertexIndex],
                                                        normals[vi1.NormalIndex],
                                                        vi1.S,
                                                        vi1.T
                                                    );

                                                    const auto& vi2 = triangleVertices[vi];
                                                    R_ASSERT(vi2.VertexIndex < vertices.size(), "Vertex of triangle fan is out of bounds");
                                                    R_ASSERT(vi2.NormalIndex < normals.size(), "Normal of triangle fan is out of bounds");
                                                    meshVertices.emplace_back(
                                                        vertices[vi2.VertexIndex],
                                                        normals[vi2.NormalIndex],
                                                        vi2.S,
                                                        vi2.T
                                                    );
                                                }
                                            }
                                            else // count < 0 // Triangle Fan
                                            {
                                                //std::cout << "# " << ti << " Triangle Fan (" << count_abs << ")" << std::endl;

                                                const auto& vi0 = triangleVertices[0];
                                                R_ASSERT(vi0.VertexIndex < vertices.size(), "First vertex of triangle fan is out of bounds");
                                                R_ASSERT(vi0.NormalIndex < normals.size(), "First normal of triangle fan is out of bounds");
                                                MdlFile::Vertex v0 = {
                                                    vertices[vi0.VertexIndex],
                                                    normals[vi0.NormalIndex],
                                                    vi0.S,
                                                    vi0.T
                                                };

                                                meshVertices.reserve((count_abs - 2) * 3);
                                                for(int vi = 2; vi < count_abs; vi++)
                                                {
                                                    meshVertices.emplace_back(v0);

                                                    const auto& vi1 = triangleVertices[vi - 1];
                                                    R_ASSERT(vi1.VertexIndex < vertices.size(), "Vertex of triangle fan is out of bounds");
                                                    R_ASSERT(vi1.NormalIndex < normals.size(), "Normal of triangle fan is out of bounds");
                                                    meshVertices.emplace_back(
                                                        vertices[vi1.VertexIndex],
                                                        normals[vi1.NormalIndex],
                                                        vi1.S,
                                                        vi1.T
                                                    );

                                                    const auto& vi2 = triangleVertices[vi];
                                                    R_ASSERT(vi2.VertexIndex < vertices.size(), "Vertex of triangle fan is out of bounds");
                                                    R_ASSERT(vi2.NormalIndex < normals.size(), "Normal of triangle fan is out of bounds");
                                                    meshVertices.emplace_back(
                                                        vertices[vi2.VertexIndex],
                                                        normals[vi2.NormalIndex],
                                                        vi2.S,
                                                        vi2.T
                                                    );
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            outBodyPart.Models.emplace_back(outModel);
                        }

                    }
                }
                mdl.BodyParts.emplace_back(outBodyPart);
            }
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.AttachmentCount)
        {
            mdl.Attachments.resize(basicInfo.AttachmentCount);
            in.seekg(basicInfo.AttachmentIndex, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.Attachments.data()), sizeof(MdlFile::Attachment) * mdl.Attachments.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        //TODO Sound
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const MdlFile& mdl)
    {
        throw std::runtime_error("Not Implemented"); //TODO
    }
}
