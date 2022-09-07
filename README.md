# Decay Library

Library for parsing BSP and WAD files from GoldSource (and maybe more later).

Also contains C API which can be used from other programming languages.
[C# wrapper](https://github.com/AbitTheGray/Decay-Library_Csharp) is being worked on (for future use in Unity3D) but may lag-behind in features.

Can also be used as Command-line program.

## Libraries

| Name                                   | License                                                                    | Version                                                         |
|----------------------------------------|----------------------------------------------------------------------------|-----------------------------------------------------------------|
| [GLM](https://glm.g-truc.net)          | [MIT](https://glm.g-truc.net/copying.txt)                                  | branch: [`master`](https://github.com/g-truc/glm/tree/master)   |
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
    - Fonts
      
## Compile & Use

Compilation tested using GCC and MinGW, C++20 support required.

Look into `tests` directory for different usages.

All tests expect `half-life` directory (the one containing `valve`, `cstrike`, `platform`...) in project directory (same as `lib`, `src`...).
It is recommended to keep it where it is and use [Symbolic link](https://en.wikipedia.org/wiki/Symbolic_link).

### Command-line utility

Enabled by default, `set(DECAY_LIBRARY_CMD=OFF)` in your `CMakeLists.txt` to disable.

- `help` = display information how to use the command-line utility.

#### BSP

- ~~`bsp_optimize <map.bsp>` - Decreases size of BSP file~~
  - At this time, `bsp2wad` can be used to minimize BSP file but generated WAD file (it may not create one if the BSP does not have packed textures)
- `bsp2obj <map.bsp> [file.obj] [file.mtl] [textures_dir=`file.mtl`/../textures]` = converts BSP to OBJ + MTD, exports textures into own directory.
  - Used textures without data are exported only as placeholders (with correct dimensions).
- ~~`bsp2wad <map.bsp> [map.wad] [new_map.bsp]`~~ - Extracts textures from BSP to WAD
  - If `new_map.bsp` is supplied, new BSP is created without those textures (only referenced, not packed)
    - `new_map.bsp` **DOES NOT** reference the `map.wad` file!
- ~~`bsp2png <map.bsp> <texture_dir=.>` - Extracts textures from BSP to PNG files~~
- `bsp_lightmap <map.bsp> [lightmap.png]` = extracts per-face lightmap and packs them into few big lightmaps
  - Big lightmap(s) have "holes" (unused pixels)
- ~~`bsp_wadref <map.bsp> <wad...` - Search `<wad...` (files & dirs) for textures and unpack them from BSP where possible~~
  - ~~This is reverse action to `-nowadtextures` or `-wadinclude` for [`hlcsg.exe`](http://zhlt.info/command-reference.html#hlcsg)~~
  - ~~Also acts like `-wadautodetect` and removes reference to unused WAD files~~
    - ~~Won't do anything if there are textures which could not be found in provided WAD files~~
  - ~~Prints all used WADs into standard output, including list of used textures from the WAD~~
- ~~`bsp_entity_extract <map.bsp> <entities.txt>` - Extracts entities to text document~~
  - ~~Total number of entities is written into standard output~~
- ~~`bsp_entity_apply <map.bsp> <entities.txt>` - Applies entities back into BSP (after `bsp_entity`)~~
- ~~`bsp_entity_validate <map.bsp/entities.txt> <gamemode.fgd>` - Validates entities against FGD file~~

#### WAD

- ~~`wad_optimize <file.wad>` - Decreases size of WAD file~~
- `wad_add <file.wad> <texture...` = add texture(s) into WAD
  - Does not parse textures, only WAD header

#### SPR

- ~~`spr2png <file.spr>` - Converts sprite to PNG~~

#### MDL

- ~~`mdl2obj <file.mdl> [file.obj] [file.mtl] [pose] [pose_time=0]` - Converts model to OBJ, optionally at specified pose (and time)~~
  - ~~Prints all available poses into standard output~~

#### FGD

- ~~`fgd2json <file.fgd>`~~
  - ~~Converts all entities to JSON structure, implements base classes~~

## Linux tools

Inside `linux` directory, there are [MIME type](https://en.wikipedia.org/wiki/Media_type#Mime.types) definitions for supported BSP and WAD files for [KDE](https://kde.org/).

They recommend having only 1 `.xml` file per application but several are used sor simplicity.
All have weight `80` (same as `application/x-doom`) but are defined by file headers.

| MIME                        | Definition File     | File type      |
|-----------------------------|---------------------|----------------|
| `application/goldsrc-bsp30` | `goldsrc-bsp30.xml` | BSP version 30 |
| `application/goldsrc-wad2`  | `goldsrc-wad2.xml`  | WAD2           |
| `application/goldsrc-wad3`  | `goldsrc-wad3.xml`  | WAD3           |

## Game Engines

Decay Library does not support and is not supported by any game engine.

In the future, it is planned to use [C# wrapper](https://github.com/AbitTheGray/Decay-Library_Csharp) for [Unity3D](https://unity.com/) (with additional scripts to create, for example, working doors).

Version for [Unreal Engine](https://www.unrealengine.com/en-US/) should be possible but is currently not planned.
If (or When) it will be in making, it will contain both C++ and Blueprint access.
