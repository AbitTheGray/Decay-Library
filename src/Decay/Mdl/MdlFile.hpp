#pragma once

#include "Decay/Common.hpp"

#include "Decay/Wad/Wad3/WadFile.hpp"

namespace Decay::Mdl
{
    class MdlFile
    {
    public:
        enum class FileVersion
        {
            Quake = 6,
            Quake2 = 8,
            GoldSrc = 10,
            // Source = 44 to 49
        };

    public:
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
            uint32_t BoneControllerIndex{};

            uint32_t HitboxCount{};
            uint32_t HitboxIndex{};

            uint32_t SeqCount{};
            uint32_t SeqIndex{};

            uint32_t SeqGroupCount{};
            uint32_t SeqGroupIndex{};

            uint32_t TextureCount{};
            uint32_t TextureIndex{};
            uint32_t TextureDataIndex{};

            uint32_t SkinRefCount{};
            uint32_t SkinFamilyCount{};
            uint32_t SkinIndex{};

            uint32_t BodypartCount{};
            uint32_t BodypartIndex{};

            uint32_t AttachmentCount{};
            uint32_t AttachmentIndex{};

            uint32_t SoundTable{};
            uint32_t SoundIndex{};

            uint32_t SoundGroups{};
            uint32_t SoundGroupIndex{};

            uint32_t TransitionCount{};
            uint32_t TransitionIndex{};

            [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
            inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }
        };
    public:
        struct BasicInfo
        {
            std::string Name{};
            int32_t Size{};
            glm::vec3 EyePosition{};

            glm::vec3 Min{};
            glm::vec3 Max{};

            glm::vec3 BBMin{};
            glm::vec3 BBMax{};

            uint32_t Flags{};
        };

    public:
        /// bones
        struct Bone
        {
            char Name[32]; ///< Bone name for symbolic links
            int Parent; ///< Parent bone
            int Flags;
            int BoneController[6]; ///< Bone controller index, -1 == none
            glm::vec3 Position[2]; ///< Default Degree-of-Freedom values
            glm::vec3 Rotation[2]; ///< Scale for delta Degree-of-Freedom values
        };
        /// bone controllers
        struct BoneController
        {
            int BoneIndex; ///< -1 == 0
            int Type; ///< X, Y, Z, XR, YR, ZR, M
            float Start;
            float End;
            int Rest; ///< byte index value at rest
            int Index; ///< 0-3 user set controller, 4 mouth
        };
        /// intersection boxes
        struct BBox
        {
            int Bone;
            int Group; ///< intersection group

            // bounding box
            glm::vec3 BB_Max;
            glm::vec3 BB_Min;
        };
        /// demand loaded sequence groups
        struct SeqGroup
        {
            char Label[32]; ///< textual name
            char Name[64]; ///< file name
            int Unused1; ///< was "cache"  - index pointer
            int Unused2; ///< was "data" -  hack for group 0
        };
        /// sequence descriptions
        struct SeqDescription
        {
            char Label[32]; ///< sequence label

            float Fps; ///< frames per second
            int Flags; ///< looping/non-looping flags

            int Activity;
            int ActivityWeight;

            int EventCount;
            int EventIndex;

            int FrameCount; ///< number of frames per sequence

            int PivotCount; ///< number of foot pivots
            int PivotIndex;

            int MotionType;
            int MotionBone;
            glm::vec3 LinearMovement;
            int AutomovePosIndex;
            int AutomoveAngleIndex;

            glm::vec3 BB_Min; ///< per sequence bounding box
            glm::vec3 BB_Max;

            int BlendCount;
            int AnimationIndex; ///< mstudioanim_t pointer relative to start of sequence group data
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
        };

        //TODO mstudioevent_t

        /// pivots
        struct Pivot
        {
            glm::vec3 Org; ///< pivot point
            int Start;
            int End;
        };
        /// attachment
        struct Attachment
        {
            char Name[32];
            int Type;
            int Bone;
            glm::vec3 Org; ///< attachment point
            glm::vec3 Vectors[3];
        };
        struct Anim
        {
            unsigned short	offset[6];
        };
        /// animation frames
        union AnimValue
        {
            struct {
                uint8_t Valid;
                uint8_t Total;
            } Num;
            int16_t Value;
        };
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

        enum class TextureFlag : int
        {
            Chrome = 2,
            Additive = 32,
            Transparent = 64
        };
        /// skin info
        struct Raw_Texture
        {
            static const int Name_Length = 64;

            char Name[Name_Length];
            TextureFlag Flags;
            int Width;
            int Height;
            int Index;

            [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
            inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }
        };

        // skin families
        // short    index[skinfamilies][skinref]

        // studio models
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


        // vec3_t	boundingbox[model][bone][2];	// complex intersection info

        /// meshes
        struct Raw_Mesh
        {
            int TriangleCount;
            int TriangleIndex;

            int TextureID;

            int NormalCount; ///< per mesh normals
            int NormalIndex; ///< normal vec3_t
        };
        struct Raw_Vertex
        {
            int16_t VertexIndex;
            int16_t NormalIndex;
            int16_t S;
            int16_t T;
        };
        struct Vertex
        {
            glm::vec3 Vertex;
            glm::vec3 Normal;
            int16_t S;
            int16_t T;
            //TODO Vertex index for Bones
        };

        struct Model
        {
            std::string Name{};
            int Type{};
            float BoundingRadius{};

            std::map<int, std::vector<Vertex>> Meshes{};
        };
        struct BodyPart
        {
            std::string Name{};
            int Base{};
            std::vector<Model> Models{};
        };
        typedef Wad::Wad3::WadFile::Image Texture;

    public:
        static constexpr std::array<uint8_t, 4> Magic1 = { 'I', 'D', 'S', 'Q' };
        //static constexpr std::array<uint8_t, 4> Magic2 = { 'I', 'D', 'S', 'T' };
        static constexpr int32_t FormatVersion = 10;

        /*
    public:
        std::vector<VisGroup> VisGroups{};
        World WorldInfo{};

    public:
        int ActiveCamera = -1;
        std::vector<Camera> Cameras{};
         */

    public:
        BasicInfo Info;
        std::vector<Bone> Bones{};
        std::vector<BoneController> BoneControllers{};
        std::vector<BBox> Hitboxes{};
        std::vector<SeqDescription> SeqDescriptions{};
        std::vector<SeqGroup> SeqGroups{};
        std::vector<std::pair<std::string, Texture>> Textures{};
        std::vector<BodyPart> BodyParts{};
        std::vector<Attachment> Attachments{};

    public:
        MdlFile() = default;
        explicit MdlFile(std::istream& in);

        ~MdlFile() = default;
    };

    std::istream& operator>>(std::istream& in, MdlFile::Raw_BasicInfo&);
    std::ostream& operator<<(std::ostream& out, const MdlFile::Raw_BasicInfo&);

    std::istream& operator>>(std::istream& in, MdlFile&);
    std::ostream& operator<<(std::ostream& out, const MdlFile&);
}
