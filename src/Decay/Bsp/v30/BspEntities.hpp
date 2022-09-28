#pragma once

#ifdef DECAY_JSON_LIB
#   include "nlohmann/json.hpp"
#endif

#include "Decay/Bsp/v30/BspFile.hpp"

namespace Decay::Bsp::v30
{
    class BspEntities
    {
    public:
        explicit BspEntities()
        {
        }
        explicit BspEntities(std::istream& in)
          : Entities(ParseEntities(in))
        {
            ProcessIntoFastAccess();
        }
        explicit BspEntities(char* begin, char* end)
        {
            MemoryBuffer buffer(begin, end);
            std::istream in(&buffer);
            Entities = ParseEntities(in);

            ProcessIntoFastAccess();
        }
        explicit BspEntities(char* begin, std::size_t size)
        {
            MemoryBuffer buffer(begin, size);
            std::istream in(&buffer);
            Entities = ParseEntities(in);

            ProcessIntoFastAccess();
        }
        explicit BspEntities(BspFile& bsp) : BspEntities(bsp.GetRawEntityChars(), bsp.GetEntityCharCount())
        {
        }
#ifdef DECAY_JSON_LIB
        explicit BspEntities(const nlohmann::json&);
#endif

    public:
        typedef std::map<std::string, std::string> Entity;
    private:
        std::vector<Entity> Entities{}; //TODO Remove copies in `Entities_*` variables
        std::map<int, Entity> Entities_Model{};
        std::map<std::string, std::vector<Entity>> Entities_Name{};
        std::map<std::string, std::vector<Entity>> Entities_Type{};
    private:
        void ProcessIntoFastAccess();

    public:
        [[nodiscard]] inline std::size_t size() const noexcept { return Entities.size(); }
        [[nodiscard]] inline const Entity& operator[](std::size_t index) const noexcept { return Entities[index]; }
        [[nodiscard]] inline       Entity& operator[](std::size_t index)       noexcept { return Entities[index]; }
        void emplace(const Entity&);

    public:
        [[deprecated("Not fully implemented, use nlohmann::json variant instead")]]
        void ExportJson(const std::filesystem::path& filename) const;
#ifdef DECAY_JSON_LIB
        [[nodiscard]] nlohmann::json AsJson() const;
#endif

    public:
        /// Parse raw entities string into vector of entities.
        static std::vector<std::map<std::string, std::string>> ParseEntities(std::istream& in);
    };

    std::ostream& operator<<(std::ostream& out, const BspEntities&);
}
