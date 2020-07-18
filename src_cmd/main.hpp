#pragma once

#include <Decay/Bsp/BspFile.hpp>
#include <Decay/Bsp/BspTree.hpp>

#include <Decay/Wad/WadFile.hpp>

#include <cstring>

struct Command
{
    int (*Exec)(int argc, const char** argv);
    std::string Help_Params;
    std::string Help_Description;
};

int Exec_help(int argc, const char** argv);

int Exec_bsp2obj(int argc, const char** argv);

int Exec_wad_add(int argc, const char** argv);

int Exec_lightmap(int argc, const char** argv);
