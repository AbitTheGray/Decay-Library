#include "Decay/Fgd/FgdFile.hpp"

int main()
{
    using namespace Decay::Fgd;

    FgdFile fgd{};
    fgd.Classes.emplace(
        "a1",
        FgdFile::Class{
            "BaseClass",
            {},
            "a1"
        }
    );
    fgd.Classes.emplace(
        "a2",
        FgdFile::Class{
            "BaseClass",
            {},
            "a2"
        }
    );
    fgd.Classes.emplace(
        "a3",
        FgdFile::Class{
            "BaseClass",
            {
                {
                    "base",
                    {
                        { "a1", false }
                    }
                }
            },
            "a3"
        }
    );
    fgd.Classes.emplace(
        "b1",
        FgdFile::Class{
            "PointClass",
            {
                {
                    "base",
                    {
                        { "a1", false }
                    }
                }
            },
            "b1"
        }
    );
    fgd.Classes.emplace(
        "b2",
        FgdFile::Class{
            "PointClass",
            {
                {
                    "base",
                    {
                        { "a2", false }
                    }
                }
            },
            "b2"
        }
    );
    fgd.Classes.emplace(
        "b3",
        FgdFile::Class{
            "PointClass",
            {
                {
                    "base",
                    {
                        { "a3", false }
                    }
                }
            },
            "b3"
        }
    );
    fgd.Classes.emplace(
        "b4",
        FgdFile::Class{
            "PointClass",
            {
                {
                    "base",
                    {
                        { "a1", false },
                        { "a3", false }
                    }
                }
            },
            "b4"
        }
    );

    auto orderedClasses = fgd.OrderClassesByDependency();
    /*
    R_ASSERT(orderedClasses[0] == "a1");
    R_ASSERT(orderedClasses[1] == "a2");
    R_ASSERT(orderedClasses[2] == "a3"); // a1
    R_ASSERT(orderedClasses[3] == "b1"); // a1
    R_ASSERT(orderedClasses[4] == "b2"); // a2
    R_ASSERT(orderedClasses[5] == "b3"); // a3 (+a1)
    R_ASSERT(orderedClasses[6] == "b4"); // a1, a3
    */
    auto it_a1 = std::find(orderedClasses.begin(), orderedClasses.end(), "a1");
    auto it_a2 = std::find(orderedClasses.begin(), orderedClasses.end(), "a2");
    auto it_a3 = std::find(orderedClasses.begin(), orderedClasses.end(), "a3");
    auto it_b1 = std::find(orderedClasses.begin(), orderedClasses.end(), "b1");
    auto it_b2 = std::find(orderedClasses.begin(), orderedClasses.end(), "b2");
    auto it_b3 = std::find(orderedClasses.begin(), orderedClasses.end(), "b3");
    auto it_b4 = std::find(orderedClasses.begin(), orderedClasses.end(), "b4");
    R_ASSERT(it_a1 != orderedClasses.end(), "`a1` class should exist");
    R_ASSERT(it_a2 != orderedClasses.end(), "`a2` class should exist");
    R_ASSERT(it_a3 != orderedClasses.end(), "`a3` class should exist");
    R_ASSERT(it_b1 != orderedClasses.end(), "`b1` class should exist");
    R_ASSERT(it_b2 != orderedClasses.end(), "`b2` class should exist");
    R_ASSERT(it_b3 != orderedClasses.end(), "`b3` class should exist");
    R_ASSERT(it_b4 != orderedClasses.end(), "`b4` class should exist");
    R_ASSERT(orderedClasses.size() == 7, "`OrderClassesByDependency` returned incorrect number of classes");

    R_ASSERT(it_a1 < it_a3, "a1 should be before a3");
    R_ASSERT(it_a1 < it_b1, "a1 should be before b1");
    R_ASSERT(it_a2 < it_b2, "a2 should be before b2");
    R_ASSERT(it_a1 < it_b3, "a1 should be before b3");
    R_ASSERT(it_a3 < it_b3, "a3 should be before b3");
    R_ASSERT(it_a1 < it_b4, "a1 should be before b4");
    R_ASSERT(it_a3 < it_b4, "a3 should be before b4");
}
