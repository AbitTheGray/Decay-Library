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

namespace Decay::Mdl
{
    struct Raw_BasicInfo
    {
        static const int Name_Length = 64;

        char Name[Name_Length]{};
        int32_t Size{};
        glm::vec3 EyePosition{};

        glm::vec3 Min{};
        glm::vec3 Max{};

        glm::vec3 BBMin{};
        glm::vec3 BBMax{};

        uint32_t Flags{};

        uint32_t BoneCount{};
        uint32_t BoneIndex{};

        uint32_t BoneControllerCount{};
        uint32_t BoneControllerOffset{};

        uint32_t HitboxCount{};
        uint32_t HitboxOffset{};

        uint32_t SeqCount{};
        uint32_t SeqOffset{};

        uint32_t SeqGroupCount{};
        uint32_t SeqGroupOffset{};

        uint32_t TextureCount{};
        uint32_t TextureOffset{};
        uint32_t TextureDataOffset{};

        uint32_t SkinRefCount{};
        uint32_t SkinFamilyCount{};
        uint32_t SkinOffset{};

        uint32_t BodypartCount{};
        uint32_t BodypartOffset{};

        uint32_t AttachmentCount{};
        uint32_t AttachmentOffset{};

        uint32_t SoundTable{};
        uint32_t SoundOffset{};

        uint32_t SoundGroupCount{};
        uint32_t SoundGroupOffset{};

        uint32_t TransitionCount{};
        uint32_t TransitionOffset{};

        [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
        inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }
    };
    std::istream& operator>>(std::istream& in, Raw_BasicInfo&);
    std::ostream& operator<<(std::ostream& out, const Raw_BasicInfo&);
    /// body part index
    struct Raw_BodyPart
    {
        static const int Name_Length = 64;

        char Name[Name_Length];
        int ModelCount;
        int Base;
        int ModelIndex; ///< index into models array

        [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
        inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }
    };
    /// skin info
    struct Raw_Texture
    {
        static const int Name_Length = 64;

        char Name[Name_Length];
        MdlFile::TextureFlag Flags; //TODO Add to MldFile::Texture
        int Width;
        int Height;
        int Index;

        [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
        inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }
    };
    struct Raw_Vertex
    {
        int16_t VertexIndex;
        int16_t NormalIndex;
        int16_t S;
        int16_t T;
    };
    /// meshes
    struct Raw_Mesh
    {
        int TriangleCount;
        int TriangleIndex;

        int TextureID;

        int NormalCount; ///< per mesh normals
        int NormalIndex; ///< normal vec3_t
    };
    /// studio models
    struct Raw_Model
    {
        static const int Name_Length = 64;

        char Name[Name_Length];

        int Type;

        float BoundingRadius;

        int MeshCount;
        int MeshIndex;

        int VertexCount;     ///< number of unique vertices
        int VertexInfoIndex; ///< vertex bone info
        int VertexIndex;     ///< vertex vec3_t

        int NormalCount;     ///< number of unique surface normals
        int NormalInfoIndex; ///< normal bone info
        int NormalIndex;     ///< normal vec3_t

        int GroupCount; ///< deformation groups
        int GroupIndex;

        [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
        inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }
    };
    /// sequence descriptions
    struct Raw_SeqDescription
    {
        static const int Label_Length = 32;

        char Label[Label_Length]; ///< sequence label

        float Fps; ///< frames per second
        int Flags; ///< looping/non-looping flags

        int Activity;
        int ActivityWeight;

        int EventCount;
        int EventOffset;

        int FrameCount; ///< number of frames per sequence

        int PivotCount; ///< number of foot pivots
        int PivotOffset;

        int MotionType;
        int MotionBone;
        glm::vec3 LinearMovement;
        int AutomovePosIndex;
        int AutomoveAngleIndex;

        glm::vec3 BB_Min; ///< per sequence bounding box
        glm::vec3 BB_Max;

        int BlendCount;
        int AnimationOffset; ///< mstudioanim_t pointer relative to start of sequence group data
        // [blend][bone][X, Y, Z, XR, YR, ZR]

        int BlendType[2]; ///< X, Y, Z, XR, YR, ZR
        float BlendStart[2]; ///< starting value
        float BlendEnd[2]; ///< ending value
        int BlendParent;

        int SeqGroup; ///< sequence group for demand loading

        int EntryNode; ///< transition node at entry
        int ExitNode; ///< transition node at exit
        int NodeFlags; ///< transition rules

        int NextSeq; ///< auto advancing sequences


        [[nodiscard]] inline std::string Label_str() const { return Cstr2Str(Label, Label_Length); }
        inline void Label_str(const std::string& val) { Str2Cstr(val, Label, Label_Length); }
    };
}

// Stream Operators
namespace Decay::Mdl
{
    std::istream& operator>>(std::istream& in, Raw_BasicInfo& visGroup)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        in.read(reinterpret_cast<char*>(&visGroup), sizeof(Raw_BasicInfo));

        R_ASSERT(IsNullTerminated(visGroup.Name, Raw_BasicInfo::Name_Length), "VisGroup name must end with NULL character");

        //throw std::runtime_error("Not Implemented"); //TODO

        return in;
    }
    std::ostream& operator<<(std::ostream& out, const Raw_BasicInfo& visGroup)
    {
        out.write(reinterpret_cast<const char*>(&visGroup), sizeof(Raw_BasicInfo));

        return out;
    }

    std::istream& operator>>(std::istream& in, MdlFile& mdl)
    {
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        { // Magic
            char magic[4];
            in.read(magic, sizeof(magic));

            if(
                magic[0] == MdlFile::Magic1[0] ||
                magic[1] == MdlFile::Magic1[1] ||
                magic[2] == MdlFile::Magic1[2] ||
                magic[3] == MdlFile::Magic1[3]
            )
            {
#ifdef DEBUG
                std::cout << magic[0] << magic[1] << magic[2] << magic[3] << std::endl;
#endif
            }
            else if(
                magic[0] == MdlFile::Magic2[0] ||
                magic[1] == MdlFile::Magic2[1] ||
                magic[2] == MdlFile::Magic2[2] ||
                magic[3] == MdlFile::Magic2[3]
            )
            {
#ifdef DEBUG
                std::cout << magic[0] << magic[1] << magic[2] << magic[3] << std::endl;
#endif
                throw std::runtime_error("IDST is not supported");
            }
            else
            {
                throw std::runtime_error("File not recognized as MDL file");
            }

        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");
        { // Version
            int version{};
            in.read(reinterpret_cast<char*>(&version), sizeof(version));

            R_ASSERT(version == 10, "Invalid file version");
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        Raw_BasicInfo basicInfo{};
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
            in.seekg(basicInfo.BoneControllerOffset, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.BoneControllers.data()), sizeof(MdlFile::BoneController) * mdl.BoneControllers.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.HitboxCount)
        {
            mdl.Hitboxes.resize(basicInfo.HitboxCount);
            in.seekg(basicInfo.HitboxOffset, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.Hitboxes.data()), sizeof(MdlFile::BBox) * mdl.Hitboxes.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.SeqCount)
        {
            std::vector<Raw_SeqDescription> sequences(basicInfo.SeqCount);
            in.seekg(basicInfo.SeqOffset, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(sequences.data()), sizeof(Raw_SeqDescription) * sequences.size());

            for(const auto& sequence : sequences)
            {
#ifdef DEBUG
                std::cout << "Sequence: " << sequence.Label_str() << ", Frames: " << sequence.FrameCount << ", Blend count: " << sequence.BlendCount << ", Bones: " << basicInfo.BoneCount << std::endl;
#endif

                MdlFile::Sequence& seq = mdl.Sequences.emplace_back();
                seq.Label = sequence.Label_str();
                seq.Fps = sequence.Fps;
                seq.Flags = sequence.Flags;

                seq.Activity = sequence.Activity;
                seq.ActivityWeight = sequence.ActivityWeight;

                seq.FrameCount = sequence.FrameCount;

                seq.MotionType = sequence.MotionType;
                seq.MotionBone = sequence.MotionBone;
                seq.LinearMovement = sequence.LinearMovement;
                seq.AutomovePosIndex = sequence.AutomovePosIndex;
                seq.AutomoveAngleIndex = sequence.AutomoveAngleIndex;

                seq.BB_Min = sequence.BB_Min;
                seq.BB_Max = sequence.BB_Max;

                seq.BlendType[0] = sequence.BlendType[0];
                seq.BlendType[1] = sequence.BlendType[1];
                seq.BlendStart[0] = sequence.BlendStart[0];
                seq.BlendStart[1] = sequence.BlendStart[1];
                seq.BlendEnd[0] = sequence.BlendEnd[0];
                seq.BlendEnd[1] = sequence.BlendEnd[1];
                seq.BlendParent = sequence.BlendParent;

                seq.SeqGroup = sequence.SeqGroup;

                seq.EntryNode = sequence.EntryNode;
                seq.ExitNode = sequence.ExitNode;
                seq.NodeFlags = sequence.NodeFlags;

                seq.NextSeq = sequence.NextSeq;

                if(sequence.EventCount)
                {
                    in.seekg(sequence.EventOffset, std::ios_base::beg);

                    seq.Events.resize(sequence.EventCount);
                    in.read(reinterpret_cast<char*>(seq.Events.data()), sizeof(MdlFile::Event) * seq.Events.size());

#ifdef DEBUG
                    for(const auto& event : seq.Events)
                    {
                        std::cout << " - Frame: " << event.Frame << ", Event:" << event.Event << ", Type: " << event.Type << std::endl;
                        std::cout << "   " << event.Options << std::endl;
                    }
#endif
                }

                if(sequence.PivotCount)
                {
                    throw std::runtime_error("Pivots are not supported");
                }

                if(sequence.BlendCount)
                {
                    //R_ASSERT(sequence.BlendCount == 1, "Only BlendCount=1 is supported"); //TODO Support

                    enum class ComponentOffset
                    {
                        PositionX = 0,
                        PositionY,
                        PositionZ,

                        RotationX,
                        RotationY,
                        RotationZ,

                        _Count
                    };
                    static_assert(static_cast<int>(ComponentOffset::_Count) == 6);

                    in.seekg(sequence.AnimationOffset, std::ios_base::beg);

                    std::vector<int16_t> blendOffsets(sequence.BlendCount * basicInfo.BoneCount * static_cast<int>(ComponentOffset::_Count));
                    in.read(reinterpret_cast<char*>(blendOffsets.data()), sizeof(int16_t) * blendOffsets.size());

                    seq.BoneData.resize(sequence.FrameCount * basicInfo.BoneCount);

                    for(int blendIndex = 0; blendIndex < sequence.BlendCount; blendIndex++)
                    {
                        for(int boneId = 0; boneId < basicInfo.BoneCount; boneId++)
                        {
                            R_ASSERT(boneId < mdl.Bones.size(), "Bone index is not valid");
                            const auto& bone = mdl.Bones[boneId];

                            std::array<std::vector<float>, static_cast<int>(ComponentOffset::_Count)> perComponentData{};
                            for(int componentIndex = 0; componentIndex < static_cast<int>(ComponentOffset::_Count); componentIndex++)
                            {
                                auto& offset = blendOffsets[boneId * static_cast<int>(ComponentOffset::_Count) + componentIndex];
                                auto& raw = perComponentData[componentIndex];
                                raw.resize(sequence.FrameCount);
                                if(offset > 0)
                                {
                                    for(int frameIndex = 0; frameIndex < sequence.FrameCount;) // Until we have enough data for whole sequence
                                    {
                                        // Last int16 of the sequence is repeated until the end of the sequence
                                        // For 2 repeated characters there is no advantage of splitting into 2 sequences (unless you are getting close to 256 values and need to split without increasing data size).
                                        // For 3+ repeated characters you should split sequences and use the repeating
                                        // 4, 7, ABCD -> ABCDDDD
                                        // 1, 5, A -> AAAAA
                                        // 2, 3, AB -> ABB
                                        // 2, 2, AB -> AB
                                        // 3, 2, ABC -> AB (is this valid? will trigger R_ASSERT below)

                                        uint8_t compressedSize, rawSize;
                                        in.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));
                                        in.read(reinterpret_cast<char*>(&rawSize), sizeof(rawSize));
                                        R_ASSERT(compressedSize > 0, "No data to decompress");
                                        R_ASSERT(rawSize > 0, "No raw data");
                                        R_ASSERT(compressedSize <= rawSize, "Compressed size cannot be smaller than raw size"); // More data before decompression - probably corrupted file
                                        R_ASSERT(frameIndex + rawSize <= raw.size(), "Too much raw data");

                                        // non-repeating part = load directly
                                        in.read(reinterpret_cast<char*>(raw.data() + frameIndex), sizeof(uint16_t) * compressedSize);

                                        if(compressedSize != rawSize) // has repeated data
                                        {
                                            std::fill(
                                                &raw[frameIndex + compressedSize], // First character after
                                                &raw[frameIndex + rawSize], // `rawSize` used as length
                                                raw[frameIndex + compressedSize - 1]
                                            );
                                        }

                                        frameIndex += rawSize;
                                    }
                                }
                            } // for(componentIndex)
#ifdef DEBUG
                            for(int componentIndex = 0; componentIndex < static_cast<int>(ComponentOffset::_Count); componentIndex++) R_ASSERT(perComponentData[componentIndex].size() == sequence.FrameCount, "Failed to load data of same length as frame count");
#endif
                            for(int frameIndex = 0; frameIndex < sequence.FrameCount; frameIndex++)
                            {
                                seq.GetBoneData(frameIndex, boneId) = {
                                    {
                                        perComponentData[static_cast<int>(ComponentOffset::PositionX)][frameIndex],
                                        perComponentData[static_cast<int>(ComponentOffset::PositionY)][frameIndex],
                                        perComponentData[static_cast<int>(ComponentOffset::PositionZ)][frameIndex]
                                    },
                                    {
                                        perComponentData[static_cast<int>(ComponentOffset::RotationX)][frameIndex],
                                        perComponentData[static_cast<int>(ComponentOffset::RotationY)][frameIndex],
                                        perComponentData[static_cast<int>(ComponentOffset::RotationZ)][frameIndex]
                                    }
                                };
                            } // for(frameIndex)

                        } // for(boneId)
                    } // for(blendIndex)
                }
            } // for(sequence)
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.SeqGroupCount)
        {
            mdl.SeqGroups.resize(basicInfo.SeqGroupCount);
            in.seekg(basicInfo.SeqGroupOffset, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(mdl.SeqGroups.data()), sizeof(MdlFile::SeqGroup) * mdl.SeqGroups.size());
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.TextureCount)
        {
            std::vector<Raw_Texture> textures(basicInfo.TextureCount);
            in.seekg(basicInfo.TextureOffset, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(textures.data()), sizeof(Raw_Texture) * textures.size());

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

        if(basicInfo.SkinRefCount && basicInfo.SkinFamilyCount)
        {
            in.seekg(basicInfo.SkinOffset, std::ios_base::beg);

            //short    index[skinfamilies][skinref]
            std::vector<int16_t> skins(basicInfo.SkinRefCount * basicInfo.SkinFamilyCount); //TODO Save
            in.read(reinterpret_cast<char*>(skins.data()), sizeof(int16_t) * skins.size());

#ifdef DEBUG
            for(int sf = 0, i = 0; sf < basicInfo.SkinFamilyCount; sf++)
                for(int sr = 0; sr < basicInfo.SkinRefCount; sr++, i++)
                    std::cout << "Skin Family " << sf << ", Skin Ref " << sr << " = " << (int)skins[i] << std::endl;
#endif
        }
        R_ASSERT(in.good(), "Input stream is not in a good shape");

        if(basicInfo.BodypartCount)
        {
            std::vector<Raw_BodyPart> bodyparts{};
            bodyparts.resize(basicInfo.BodypartCount);
            in.seekg(basicInfo.BodypartOffset, std::ios_base::beg);
            in.read(reinterpret_cast<char*>(bodyparts.data()), sizeof(Raw_BodyPart) * bodyparts.size());

            mdl.BodyParts.reserve(bodyparts.size());
            for(const auto& bodypart : bodyparts)
            {
                MdlFile::BodyPart outBodyPart{};
                {
                    outBodyPart.Name = bodypart.Name_str();
                    outBodyPart.Base = bodypart.Base;

                    // Models
                    {
                        std::vector<Raw_Model> models(bodypart.ModelCount);
                        in.seekg(bodypart.ModelIndex, std::ios_base::beg);
                        in.read(reinterpret_cast<char*>(models.data()), sizeof(Raw_Model) * models.size());
                        R_ASSERT(in.good(), "Input stream is not in a good shape");

                        outBodyPart.Models.reserve(models.size());
                        for(const auto& model : models)
                        {
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
                                    std::vector<Raw_Mesh> meshes(model.MeshCount);
                                    in.seekg(model.MeshIndex, std::ios_base::beg);
                                    in.read(reinterpret_cast<char*>(meshes.data()), sizeof(Raw_Mesh) * meshes.size());
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
                                            std::vector<Raw_Vertex> triangleVertices(count_abs);
                                            in.read(reinterpret_cast<char*>(triangleVertices.data()), sizeof(Raw_Vertex) * triangleVertices.size());
                                            R_ASSERT(in.good(), "Input stream is not in a good shape");

                                            R_ASSERT(count_abs >= 3, "Triangle fan must have at least 3 vertices");
                                            std::vector<MdlFile::Vertex>& meshVertices = outModel.Meshes[mesh.TextureID];
                                            if(count > 0) // Triangle Strip
                                            {
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
                                                        vi0.T,
                                                        verticeGroups[vi0.VertexIndex]
                                                    );

                                                    const auto& vi1 = triangleVertices[vi - 1];
                                                    R_ASSERT(vi1.VertexIndex < vertices.size(), "Vertex of triangle fan is out of bounds");
                                                    R_ASSERT(vi1.NormalIndex < normals.size(), "Normal of triangle fan is out of bounds");
                                                    meshVertices.emplace_back(
                                                        vertices[vi1.VertexIndex],
                                                        normals[vi1.NormalIndex],
                                                        vi1.S,
                                                        vi1.T,
                                                        verticeGroups[vi1.VertexIndex]
                                                    );

                                                    const auto& vi2 = triangleVertices[vi];
                                                    R_ASSERT(vi2.VertexIndex < vertices.size(), "Vertex of triangle fan is out of bounds");
                                                    R_ASSERT(vi2.NormalIndex < normals.size(), "Normal of triangle fan is out of bounds");
                                                    meshVertices.emplace_back(
                                                        vertices[vi2.VertexIndex],
                                                        normals[vi2.NormalIndex],
                                                        vi2.S,
                                                        vi2.T,
                                                        verticeGroups[vi2.VertexIndex]
                                                    );
                                                }
                                            }
                                            else // count < 0 // Triangle Fan
                                            {
                                                const auto& vi0 = triangleVertices[0];
                                                R_ASSERT(vi0.VertexIndex < vertices.size(), "First vertex of triangle fan is out of bounds");
                                                R_ASSERT(vi0.NormalIndex < normals.size(), "First normal of triangle fan is out of bounds");
                                                MdlFile::Vertex v0 = {
                                                    vertices[vi0.VertexIndex],
                                                    normals[vi0.NormalIndex],
                                                    vi0.S,
                                                    vi0.T,
                                                    verticeGroups[vi0.VertexIndex]
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
                                                        vi1.T,
                                                        verticeGroups[vi1.VertexIndex]
                                                    );

                                                    const auto& vi2 = triangleVertices[vi];
                                                    R_ASSERT(vi2.VertexIndex < vertices.size(), "Vertex of triangle fan is out of bounds");
                                                    R_ASSERT(vi2.NormalIndex < normals.size(), "Normal of triangle fan is out of bounds");
                                                    meshVertices.emplace_back(
                                                        vertices[vi2.VertexIndex],
                                                        normals[vi2.NormalIndex],
                                                        vi2.S,
                                                        vi2.T,
                                                        verticeGroups[vi2.VertexIndex]
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
            in.seekg(basicInfo.AttachmentOffset, std::ios_base::beg);
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
