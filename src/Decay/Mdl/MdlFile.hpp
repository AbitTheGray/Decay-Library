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
            int Flags; ///< X, Y, Z, XR, YR, ZR
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
        struct SequenceBoneInfo
        {
            glm::vec3 Position;
            glm::vec3 Rotation;
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
            static const int Label_Length = 32;
            static const int Name_Length = 64;

            char Label[Label_Length]; ///< textual name
            char Name[Name_Length]; ///< file name
            int Unused1; ///< was "cache"  - index pointer
            int Unused2; ///< was "data" -  hack for group 0


            [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
            inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }

            [[nodiscard]] inline std::string Label_str() const { return Cstr2Str(Label, Label_Length); }
            inline void Label_str(const std::string& val) { Str2Cstr(val, Label, Label_Length); }
        };
        struct Event
        {
            int Frame;
            int Event;
            int Type;
            char Options[64]; //TODO What can be inside?
        };
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

        enum class TextureFlag : int
        {
            Chrome = 2,
            Additive = 32,
            Transparent = 64
        };

        // skin families
        // short    index[skinfamilies][skinref]


        // vec3_t	boundingbox[model][bone][2];	// complex intersection info

        struct Vertex
        {
            glm::vec3 Vertex;
            glm::vec3 Normal;
            int16_t S;
            int16_t T;
            int16_t BoneID;
        };

        struct Model
        {
            std::string Name{};
            int Type{};
            float BoundingRadius{};

            // Key = SkinRef (TextureID from SkinFamilies)
            // Value = Triangulated data
            std::map<int, std::vector<Vertex>> Meshes{};
        };
        struct BodyPart
        {
            std::string Name{};
            int Base{};
            std::vector<Model> Models{};
        };
        typedef Wad::Wad3::WadFile::Image Texture;
        struct Sequence
        {
            std::string Label;

            float Fps;
            int Flags; ///< looping/non-looping flags

            int Activity;
            int ActivityWeight;

            std::vector<Event> Events;

            int FrameCount; ///< number of frames per sequence

            std::vector<Pivot> Pivots;

            int MotionType;
            int MotionBone;
            glm::vec3 LinearMovement;
            int AutomovePosIndex;
            int AutomoveAngleIndex;

            glm::vec3 BB_Min; ///< per sequence bounding box
            glm::vec3 BB_Max;

            std::vector<SequenceBoneInfo> BoneData{};
            [[nodiscard]] inline const SequenceBoneInfo& GetBoneData(int frame, int bone) const { return BoneData[frame + bone * FrameCount]; }
            [[nodiscard]] inline       SequenceBoneInfo& GetBoneData(int frame, int bone)       { return BoneData[frame + bone * FrameCount]; }

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

    public:
        static constexpr std::array<uint8_t, 4> Magic1 = { 'I', 'D', 'S', 'Q' };
        static constexpr std::array<uint8_t, 4> Magic2 = { 'I', 'D', 'S', 'T' };
        static constexpr int32_t FormatVersion = 10;

    public:
        BasicInfo Info;
        std::vector<Bone> Bones{};
        std::vector<BoneController> BoneControllers{};
        std::vector<BBox> Hitboxes{};
        std::vector<Sequence> Sequences{};
        std::vector<SeqGroup> SeqGroups{};
        std::vector<std::pair<std::string, Texture>> Textures{};
        std::vector<BodyPart> BodyParts{};
        std::vector<Attachment> Attachments{};

    public:
        MdlFile() = default;
        explicit MdlFile(std::istream& in);

        ~MdlFile() = default;
    };

    std::istream& operator>>(std::istream& in, MdlFile&);
    std::ostream& operator<<(std::ostream& out, const MdlFile&);
}
