#pragma once

#include "Decay/Common.hpp"

struct Command
{
    int (*Exec)(int argc, const char** argv);
    int (*HelpExec)(int argc, const char** argv);
    std::string Help_Description;
};

int Exec_help(int argc, const char** argv);

int Exec_bsp2obj(int argc, const char** argv);
int Help_bsp2obj(int argc, const char** argv);

int Exec_bsp2wad(int argc, const char** argv);
int Help_bsp2wad(int argc, const char** argv);

int Exec_wad_add(int argc, const char** argv);
int Help_wad_add(int argc, const char** argv);

int Exec_wad(int argc, const char** argv);
int Help_wad(int argc, const char** argv);

int Exec_bsp_lightmap(int argc, const char** argv);
int Help_bsp_lightmap(int argc, const char** argv);

int Exec_bsp_entity(int argc, const char** argv);
int Help_bsp_entity(int argc, const char** argv);

int Exec_map2rmf(int argc, const char** argv);
int Help_map2rmf(int argc, const char** argv);

int Exec_rmf2map(int argc, const char** argv);
int Help_rmf2map(int argc, const char** argv);

int Exec_fgd(int argc, const char** argv);
int Help_fgd(int argc, const char** argv);
