#pragma once

#include <filesystem>
#include <vector>
#include <map>
#include "glm/glm.hpp"

#include "Decay/Common.hpp"

#include "Decay/Common.hpp"
namespace Decay::Fdg
{
    /// Raw FGD file data, only basic validation is done on it.
    class FdgFile
    {
    public:
        enum class EntryType
        {
            BaseClass, // @BaseClass
            SolidClass, // @SolidClass
            PointClass, // @PointClass
        };
        enum class EntityItemType // Case Insensitive (at least 1st char)
        {
            String, // string
            Integer, // integer
            Choices, // choices
            Flags, // flags
            Target_Source, // target_source
            Target_Destination, // target_destination
            Color255, // color255
            Sound, // sound
            Studio, // studio (what to show in the editor)
            Sprite, // sprite
            Decal, // decal
        };

        struct EntityItemOption
        {
            /// `choices`: New value to set
            /// `flags`: Value to add or subtract, for bit index use `FlagOrder()`
            int16_t     Index;
            /// Display text
            std::string DisplayName;
            /// Valid only for `flags`
            bool        Default = false;

            /// Returns index of the flag for correct ordering.
            /// -1 = Index out of bounds (<= 0)
            /// -2 = Unable to match single flag
            inline int16_t FlagOrder() noexcept
            {
                if(Index <= 0)
                    return -1;
                for(int i = 0; i < sizeof(decltype(Index)) * 8; i++)
                {
                    if(Index == (1 << i))
                        return i;
                }
                return -2;
            }
        };
        struct EntityItem
        {
            std::string Codename;
            std::string Type;
            std::string DisplayName;
            std::string DefaultValue; //TODO Create class for std::optional<int or std::string>
            std::vector<EntityItemOption> Options;
        };
        struct Entity
        {
            EntryType Type;
            std::string Codename;
            std::string DisplayName;
            std::vector<std::string> BaseClasses;

            // `size`
            glm::i32vec3 BB_Min = { 0, 0, 0 };
            glm::i32vec3 BB_Max = { 0, 0, 0 };
            // `color`
            glm::u8vec3 Color = { 0xFF, 0xFF, 0xFF };

            std::vector<EntityItem> Items;
        };

    public:
        FdgFile(const std::filesystem::path& filename);
        ~FdgFile() = default;

    public:
        std::vector<Entity> Entities = {};

    public:
        /// Returns: key=text, value=values in round brackets
        static std::pair<std::string, std::vector<std::string>> ReadEntityHeaderWord(std::istream& in);
    };

    template<typename T>
    inline T to_enum(std::string_view str) = delete;

    inline consteval const char* to_string(FdgFile::EntryType et) noexcept
    {
        switch(et)
        {
            default: return nullptr;
            case FdgFile::EntryType::BaseClass:  return "@BaseClass";
            case FdgFile::EntryType::SolidClass: return "@SolidClass";
            case FdgFile::EntryType::PointClass: return "@PointClass";
        }
    }
    template<>
    inline FdgFile::EntryType to_enum<FdgFile::EntryType>(std::string_view str)
    {
        if(str.length() == 0)
            throw std::runtime_error("String cannot be empty");
        if(str[0] != '@')
            throw std::runtime_error("FDG EntryType must always start with @");
        if(str.length() < 10)
            throw std::runtime_error("FDG EntryType is too short");
        switch(str[1])
        {
            default:
                throw std::runtime_error("Invalid value");
            case 'b':
            case 'B':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntryType::BaseClass)))
                    return FdgFile::EntryType::BaseClass;
                throw std::runtime_error("Invalid BaseClass value");
            case 's':
            case 'S':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntryType::SolidClass)))
                    return FdgFile::EntryType::SolidClass;
                throw std::runtime_error("Invalid SolidClass value");
            case 'p':
            case 'P':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntryType::PointClass)))
                    return FdgFile::EntryType::PointClass;
                throw std::runtime_error("Invalid PointClass value");
        }
    }
    inline consteval const char* to_string(FdgFile::EntityItemType vt) noexcept
    {
        switch(vt)
        {
            default: return nullptr;
            case FdgFile::EntityItemType::String:             return "string";
            case FdgFile::EntityItemType::Integer:            return "integer";
            case FdgFile::EntityItemType::Choices:            return "choices";
            case FdgFile::EntityItemType::Flags:              return "flags";
            case FdgFile::EntityItemType::Target_Source:      return "target_source";
            case FdgFile::EntityItemType::Target_Destination: return "target_destination";
            case FdgFile::EntityItemType::Color255:           return "color255";
            case FdgFile::EntityItemType::Sound:              return "sound";
            case FdgFile::EntityItemType::Studio:             return "studio";
            case FdgFile::EntityItemType::Sprite:             return "sprite";
            case FdgFile::EntityItemType::Decal:              return "decal";
        }
    }
    template<>
    inline FdgFile::EntityItemType to_enum<FdgFile::EntityItemType>(std::string_view str)
    {
        if(str.length() == 0)
            throw std::runtime_error("String cannot be empty");
        if(str.length() < 5)
            throw std::runtime_error("FDG ValueType is too short");
        switch(str[1])
        {
            default:
                throw std::runtime_error("Invalid value");
            case 'c':
            case 'C':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Choices)))
                    return FdgFile::EntityItemType::Choices;
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Color255)))
                    return FdgFile::EntityItemType::Color255;
                throw std::runtime_error("Invalid value");
            case 'd':
            case 'D':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Decal)))
                    return FdgFile::EntityItemType::Decal;
                throw std::runtime_error("Invalid value");
            case 'f':
            case 'F':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Flags)))
                    return FdgFile::EntityItemType::Flags;
                throw std::runtime_error("Invalid value");
            case 'i':
            case 'I':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Integer)))
                    return FdgFile::EntityItemType::Integer;
                throw std::runtime_error("Invalid value");
            case 's':
            case 'S':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::String)))
                    return FdgFile::EntityItemType::String;
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Studio)))
                    return FdgFile::EntityItemType::Studio;
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Sprite)))
                    return FdgFile::EntityItemType::Sprite;
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Sound)))
                    return FdgFile::EntityItemType::Sound;
                throw std::runtime_error("Invalid value");
            case 't':
            case 'T':
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Target_Destination)))
                    return FdgFile::EntityItemType::Target_Destination;
                if(StringCaseInsensitiveEqual(str, to_string(FdgFile::EntityItemType::Target_Source)))
                    return FdgFile::EntityItemType::Target_Source;
                throw std::runtime_error("Invalid value");
        }
    }
}
