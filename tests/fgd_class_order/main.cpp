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

    auto classDependency = fgd.OrderClassesByDependency();
    for(const auto& cd : classDependency)
        std::cout << cd << std::endl;
}
