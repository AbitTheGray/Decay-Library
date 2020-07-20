# Decay Library

Library for parsing BSP and WAD files from GoldSource (and maybe more later).

Also contains C API which can be used from other programming languages.
[C# wrapper](https://github.com/AbitTheGray/Decay-Library_Csharp) is being worked on (for future use in Unity3D).

Can also be used as Command-line program.

## Libraries

| Name | License | Version |
|------|---------|---------|
| [GLM](https://glm.g-truc.net) | [MIT](https://glm.g-truc.net/copying.txt) | branch: [`master`](https://github.com/g-truc/glm/tree/master) |
| [stb](https://github.com/nothings/stb) | [MIT / Public Domain](https://github.com/nothings/stb/blob/master/LICENSE) | branch: [`master`](https://github.com/nothings/stb/tree/master) |

All libraries are used as `static library` to maximize optimization and limit problems with deployment and versions.

## Formats

- GoldSrc (Half-Life, Counter-Strike...)
  - BSP (v30)
    - Extract/Export:
      - Textures
        - RGB/RGBA or raw data with palette
        - ~~Generate WAD~~
      - Entity configuration
      - Map into .obj
        - Mtl with correct UV
        - ~~Separate collision map~~
      - ~~Lightmap blob as texture~~
      - Nodes with map
        - Map components
        - ~~Visibility~~
          - Simple per-face textures
      - ~~Command entities as scripts~~
  - WAD (WAD2 + WAD3)
    - Textures
      - Raw data with palette
      - RGB and RGBA export supported
    - Images
      - Some seem weird but it may be correct
    - ~~Fonts~~
      - **Currently fail to load, requires fixing**
      
## Compile & Use

Compilation tested using GCC and MinGW, C++20 support required.

Look into `tests` directory for different usages.

All tests expect `half-life` directory (the one containing `valve`, `cstrike`, `platform`...) in project directory (same as `lib`, `src`...).
It is recommended to keep it where it is and use [Symbolic link](https://en.wikipedia.org/wiki/Symbolic_link).

### Command-line utility

Enabled by default, `set(DECAY_LIBRARY_CMD=OFF)` in your `CMakeLists.txt` to disable.

- `help` = display information how to use the command-line utility.
- `bsp2obj <map.bsp> [file.obj] [file.mtl] [textures_dir=`file.mtl`/../textures]` = converts BSP to OBJ + MTD, exports textures into own directory.
  - Used textures without data are exported only as placeholders (with correct dimensions).
- ~~`bsp2wad <map.bsp> [map.wad] [new_map.bsp]`~~ - Extracts textures from BSP to WAD
  - ~~If `new_map.bsp` is supplied, new BSP is created without those textures (only referenced, not packed)~~
- `wad_add <file.wad> <texture...` = add texture(s) into WAD
  - Does not parse textures, only WAD header
- `lightmap <map.bsp> [lightmap.png]` = extracts per-face lightmap and packs them into few big lightmaps
  - Big lightmap(s) have "holes" (unused pixels)
- ~~`bsp_optimize` - Decreases size of BSP file~~
- ~~`wad_optimize` - Decreases size of WAD file~~

## Linux tools

Inside `linux` directory, there are [MIME type](https://en.wikipedia.org/wiki/Media_type#Mime.types) definitions for supported BSP and WAD files for [KDE](https://kde.org/).

They recommend having only 1 `.xml` file per application but several are used sor simplicity.
All have weight `80` (same as `application/x-doom`) but are defined by file headers.

| MIME | Definition File | File type |
|------|-----------------|-----------|
| `application/goldsrc-bsp30` | `goldsrc-bsp30.xml` | BSP version 30 |
| `application/goldsrc-wad2` | `goldsrc-wad2.xml` | WAD2 |
| `application/goldsrc-wad3` | `goldsrc-wad3.xml` | WAD3 |

## Game Engines

Decay Library does not support and is not supported by any game engine.

In future, it is planned to use [C# wrapper](https://github.com/AbitTheGray/Decay-Library_Csharp) for [Unity3D](https://unity.com/) (with additional scripts to create, for example, working doors).

Version for [Unreal Engine](https://www.unrealengine.com/en-US/) should be possible but is currently not planned.
If (or When) it will be in making, it will contain both C++ and Blueprint access.
