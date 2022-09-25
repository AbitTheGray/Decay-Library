#pragma once

#include "Decay/Common.hpp"
#include "unordered_map"

namespace Decay::Rmf
{
    /// https://developer.valvesoftware.com/wiki/Rich_Map_Format
    class RmfFile
    {
    public:
        typedef glm::vec3 Vector_t;
        typedef glm::u8vec3 Color_t;
        struct VisGroup
        {
            static const int Name_Length = 128;
            static const int Dummy2_Length = 3;

            char Name[Name_Length];
            Color_t Color;
            uint8_t Dummy;
            int Index;
            uint8_t Visible; ///< 1 = visible, 0 = not visible
            uint8_t Dummy2[Dummy2_Length];

            [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
            inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }


            [[nodiscard]] inline bool operator==(const VisGroup& other) const
            {
                for(int i = 0; i < Name_Length; i++)
                    if(Name[i] != other.Name[i])
                        return false;

                if(Color != other.Color)
                    return false;
                if(Dummy != other.Dummy)
                    return false;
                if(Index != other.Index)
                    return false;
                if(Visible != other.Visible)
                    return false;
                if(Dummy2[0] != other.Dummy2[0])
                    return false;
                if(Dummy2[1] != other.Dummy2[1])
                    return false;
                if(Dummy2[2] != other.Dummy2[2])
                    return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const VisGroup& other) const { return !operator==(other); }
        };
        static_assert(sizeof(VisGroup) == 140);
        struct Face
        {
            static const int TextureName_Length = 256;
            static const int Dummy2_Length = 16;
            static const int PlaneVertices_Length = 3;

            char TextureName[TextureName_Length];
            float Dummy;

            Vector_t UAxis;
            float XShift;
            Vector_t VAxis;
            float YShift;

            float TextureRotation; ///< Degrees
            glm::vec2 TextureScale;

            uint8_t Dummy2[Dummy2_Length];
            std::vector<Vector_t> Vertices;
            Vector_t PlaneVertices[PlaneVertices_Length];

            [[nodiscard]] inline std::string TextureName_str() const { return Cstr2Str(TextureName, TextureName_Length); }
            inline void TextureName_str(const std::string& val) { Str2Cstr(val, TextureName, TextureName_Length); }


            [[nodiscard]] inline bool operator==(const Face& other) const
            {
                for(int i = 0; i < TextureName_Length; i++)
                    if(TextureName[i] != other.TextureName[i])
                        return false;

                if(Dummy != other.Dummy)
                    return false;

                if(UAxis != other.UAxis)
                    return false;
                if(XShift != other.XShift)
                    return false;
                if(VAxis != other.VAxis)
                    return false;
                if(YShift != other.YShift)
                    return false;

                if(TextureRotation != other.TextureRotation)
                    return false;
                if(TextureScale != other.TextureScale)
                    return false;

                for(int i = 0; i < Dummy2_Length; i++)
                    if(Dummy2[i] != other.Dummy2[i])
                        return false;

                if(Vertices.size() != other.Vertices.size())
                    return false;
                for(int i = 0; i < Vertices.size(); i++)
                    if(Vertices[i] != other.Vertices[i])
                        return false;

                if(PlaneVertices[0] != other.PlaneVertices[0])
                    return false;
                if(PlaneVertices[1] != other.PlaneVertices[1])
                    return false;
                if(PlaneVertices[2] != other.PlaneVertices[2])
                    return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Face& other) const { return !operator==(other); }
        };
        struct Solid
        {
            static constexpr const char* TypeName = "CMapSolid";

            static const int Dummy_Length = 4;

            int VisGroup;
            Color_t DisplayColor;
            uint8_t Dummy[Dummy_Length];
            std::vector<Face> Faces;


            [[nodiscard]] inline bool operator==(const Solid& other) const
            {
                if(VisGroup != other.VisGroup)
                    return false;
                if(DisplayColor != other.DisplayColor)
                    return false;

                for(int i = 0; i < Dummy_Length; i++)
                    if(Dummy[i] != other.Dummy[i])
                        return false;

                if(Faces.size() != other.Faces.size())
                    return false;
                for(int i = 0; i < Faces.size(); i++)
                    if(Faces[i] != other.Faces[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Solid& other) const { return !operator==(other); }
        };
        struct Entity
        {
            static constexpr const char* TypeName = "CMapEntity";

            static const int Classname_MaxLength = 128;
            static const int Dummy_Length = 4;
            static const int KeyValue_Key_MaxLength = 32;
            static const int KeyValue_Value_MaxLength = 100;
            static const int Dummy2_Length = 14;
            static const int Dummy3_Length = 4;

            int VisGroup;
            Color_t DisplayColor;
            std::vector<Solid> Solids;
            std::string Classname;
            uint8_t Dummy[Dummy_Length];
            int EntityFlags;
            std::unordered_map<std::string, std::string> KeyValue;
            uint8_t Dummy2[Dummy2_Length];
            Vector_t Position;
            uint8_t Dummy3[Dummy3_Length];


            [[nodiscard]] inline bool operator==(const Entity& other) const
            {
                if(VisGroup != other.VisGroup)
                    return false;
                if(DisplayColor != other.DisplayColor)
                    return false;

                if(Solids.size() != other.Solids.size())
                    return false;
                for(int i = 0; i < Solids.size(); i++)
                    if(Solids[i] != other.Solids[i])
                        return false;

                if(Classname != other.Classname)
                    return false;

                for(int i = 0; i < Dummy_Length; i++)
                    if(Dummy[i] != other.Dummy[i])
                        return false;

                if(EntityFlags != other.EntityFlags)
                    return false;

                //TODO Compare values of `KeyValue`

                for(int i = 0; i < Dummy2_Length; i++)
                    if(Dummy2[i] != other.Dummy2[i])
                        return false;

                if(Position != other.Position)
                    return false;

                for(int i = 0; i < Dummy3_Length; i++)
                    if(Dummy3[i] != other.Dummy3[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Entity& other) const { return !operator==(other); }
        };
        struct Group
        {
            static constexpr const char* TypeName = "CMapGroup";

            int VisGroup;
            Color_t DisplayColor;

            // Objects
            std::vector<Solid> Solids;
            std::vector<Entity> Entities;
            std::vector<Group> Groups;


            [[nodiscard]] inline bool operator==(const Group& other) const
            {
                if(VisGroup != other.VisGroup)
                    return false;
                if(DisplayColor != other.DisplayColor)
                    return false;

                if(Solids.size() != other.Solids.size())
                    return false;
                for(int i = 0; i < Solids.size(); i++)
                    if(Solids[i] != other.Solids[i])
                        return false;

                if(Entities.size() != other.Entities.size())
                    return false;
                for(int i = 0; i < Entities.size(); i++)
                    if(Entities[i] != other.Entities[i])
                        return false;

                if(Groups.size() != other.Groups.size())
                    return false;
                for(int i = 0; i < Groups.size(); i++)
                    if(Groups[i] != other.Groups[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Group& other) const { return !operator==(other); }
        };
        struct Corner
        {
            static const int NameOverride_Length = 128;

            Vector_t Position;
            int Index; ///< Used to generate targetnames (corner01, corner02...)
            char NameOverride[NameOverride_Length]; ///< Empty for no override
            std::unordered_map<std::string, std::string> KeyValue;

            [[nodiscard]] inline std::string NameOverride_str() const { return Cstr2Str(NameOverride, NameOverride_Length); }
            inline void NameOverride_str(const std::string& val) { Str2Cstr(val, NameOverride, NameOverride_Length); }


            [[nodiscard]] inline bool operator==(const Corner& other) const
            {
                if(Position != other.Position)
                    return false;
                if(Index != other.Index)
                    return false;

                for(int i = 0; i < NameOverride_Length; i++)
                    if(NameOverride[i] != other.NameOverride[i])
                        return false;

                //TODO Compare values of `KeyValue`

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Corner& other) const { return !operator==(other); }
        };
        enum class PathType : int32_t
        {
            OneWay = 0,
            Circular = 1,
            PingPong = 2
        };
        struct Path
        {
            static const int Name_Length = 128;
            static const int Class_Length = 128;

            char Name[Name_Length]{};
            char Class[Class_Length]{};
            PathType Type{};
            std::vector<Corner> Corners{};

            [[nodiscard]] inline std::string Name_str() const { return Cstr2Str(Name, Name_Length); }
            inline void Name_str(const std::string& val) { Str2Cstr(val, Name, Name_Length); }

            [[nodiscard]] inline std::string Class_str() const { return Cstr2Str(Class, Class_Length); }
            inline void Class_str(const std::string& val) { Str2Cstr(val, Class, Class_Length); }


            [[nodiscard]] inline bool operator==(const Path& other) const
            {
                for(int i = 0; i < Name_Length; i++)
                    if(Name[i] != other.Name[i])
                        return false;

                for(int i = 0; i < Class_Length; i++)
                    if(Class[i] != other.Class[i])
                        return false;

                if(Type != other.Type)
                    return false;

                if(Corners.size() != other.Corners.size())
                    return false;
                for(int i = 0; i < Corners.size(); i++)
                    if(Corners[i] != other.Corners[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Path& other) const { return !operator==(other); }
        };
        struct Camera
        {
            Vector_t EyePosition{};
            Vector_t LookPosition{};


            [[nodiscard]] inline bool operator==(const Camera& other) const
            {
                if(EyePosition != other.EyePosition)
                    return false;
                if(LookPosition != other.LookPosition)
                    return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const Camera& other) const { return !operator==(other); }
        };
        struct World
        {
            static constexpr const char* TypeName = "CMapWorld";

            static const int Dummy_Length = 4;
            static const int Dummy2_Length = 12;

            int VisGroup{};
            Color_t DisplayColor{};

            // Objects
            std::vector<Solid> Solids{};
            std::vector<Entity> Entities{};
            std::vector<Group> Groups{};

            std::string Classname; ///< Entity.Classname
            uint8_t Dummy[Dummy_Length]{}; ///< Entity.Dummy
            int EntityFlags; ///< Entity.EntityFlags
            std::unordered_map<std::string, std::string> KeyValue; ///< Entity.KeyValue
            uint8_t Dummy2[Dummy2_Length]{};
            std::vector<Path> Paths;


            [[nodiscard]] inline bool operator==(const World& other) const
            {
                if(VisGroup != other.VisGroup)
                    return false;
                if(DisplayColor != other.DisplayColor)
                    return false;

                if(Solids.size() != other.Solids.size())
                    return false;
                for(int i = 0; i < Solids.size(); i++)
                    if(Solids[i] != other.Solids[i])
                        return false;

                if(Entities.size() != other.Entities.size())
                    return false;
                for(int i = 0; i < Entities.size(); i++)
                    if(Entities[i] != other.Entities[i])
                        return false;

                if(Groups.size() != other.Groups.size())
                    return false;
                for(int i = 0; i < Groups.size(); i++)
                    if(Groups[i] != other.Groups[i])
                        return false;

                if(Classname != other.Classname)
                    return false;

                for(int i = 0; i < Dummy_Length; i++)
                    if(Dummy[i] != other.Dummy[i])
                        return false;

                if(EntityFlags != other.EntityFlags)
                    return false;

                //TODO Compare values of `KeyValue`

                for(int i = 0; i < Dummy2_Length; i++)
                    if(Dummy2[i] != other.Dummy2[i])
                        return false;

                for(int i = 0; i < Dummy2_Length; i++)
                    if(Dummy2[i] != other.Dummy2[i])
                        return false;

                if(Paths.size() != other.Paths.size())
                    return false;
                for(int i = 0; i < Paths.size(); i++) //TODO Order-independent comparison
                    if(Paths[i] != other.Paths[i])
                        return false;

                return true;
            }
            [[nodiscard]] inline bool operator!=(const World& other) const { return !operator==(other); }
        };

    public:
        static constexpr std::array<uint8_t, 4> Magic1 = {
            0xCD, 0xCC, 0x0C, 0x40 // 2.2
        };
        static constexpr std::array<uint8_t, 3> Magic2 = { 'R', 'M', 'F' };
        static constexpr std::array<uint8_t, 8> DocInfo = { 'D', 'O', 'C', 'I', 'N', 'F', 'O', 0 };

    public:
        std::vector<VisGroup> VisGroups{};
        World WorldInfo{};

    public:
        int ActiveCamera = 0;
        std::vector<Camera> Cameras{};

    public:
        RmfFile() = default;
        explicit RmfFile(std::istream& in);
        ~RmfFile() = default;
    };

    std::istream& operator>>(std::istream& in, RmfFile::VisGroup&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::VisGroup&);

    std::istream& operator>>(std::istream& in, RmfFile::Face&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::Face&);

    /// Does not check `TypeName`
    std::istream& operator>>(std::istream& in, RmfFile::Solid&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::Solid&);

    /// Does not check `TypeName`
    std::istream& operator>>(std::istream& in, RmfFile::Entity&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::Entity&);

    /// Does not check `TypeName`
    std::istream& operator>>(std::istream& in, RmfFile::Group&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::Group&);

    std::istream& operator>>(std::istream& in, RmfFile::Corner&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::Corner&);

    std::istream& operator>>(std::istream& in, RmfFile::PathType&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::PathType&);

    std::istream& operator>>(std::istream& in, RmfFile::Path&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::Path&);

    std::istream& operator>>(std::istream& in, RmfFile::Camera&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::Camera&);

    /// Does not check `TypeName`
    std::istream& operator>>(std::istream& in, RmfFile::World&);
    std::ostream& operator<<(std::ostream& out, const RmfFile::World&);

    std::istream& operator>>(std::istream& in, RmfFile&);
    std::ostream& operator<<(std::ostream& out, const RmfFile&);
}
