#pragma once

#include <filesystem>
#include <vector>
#include <map>
#include <glm/glm.hpp>

#include "Decay/Common.hpp"

#include "Decay/Common.hpp"
namespace Decay::Fdg
{
    class FdgFile
    {
    public:
        enum class EntryType
        {
            BaseClass, // @BaseClass
            SolidClass, // @SolidClass
            PointClass, // @PointClass
        };
        enum class ValueType // Case Insensitive (at least 1st char)
        {
            String, // string
            Integer, // integer
            Choices, // choices
            Flags, // flags
            Target_Source, // target_source
            Target_Destination, // target_destination
            Color255, // color255
            Sound, // sound
            Studio, // studio (for model)
            Sprite, // sprite
            Decal, // decal
        };

        struct ChoicesItem
        {
            int16_t     Index;
            std::string Display;
        };
        struct FlagsItem
        {
            int16_t     Index;
            std::string Display;
            bool        Default = 0;
        };
    };

    inline const char* to_string(FdgFile::EntryType et)
    {

    }
    inline const char* to_string(FdgFile::ValueType vt)
    {

    }
}
