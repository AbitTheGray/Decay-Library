# Decay Library

Library for parsing BSP and WAD files from GoldSource (and later IdTech2 and Source) engine.

Also contains C API which can be used from other programming languages.
[C# wrapper](https://github.com/AbitTheGray/Decay-Library_Csharp) is being worked on (for future use in Unity3D) but may lag-behind in features.

Can also be used as Command-line program.

## Goal

Goal of this project is to learn, test and document.

Learn = get to know how others approached the problem before, what decisions they made and how it shaped further development.

Test = write a code that can interpret the data, see whenever it can be done easier today, validate files.

Document = write documentation for others and "share the knowledge" (as did people before this project).

## Libraries

| Name                                   | License                                                                    | Version                                                         |
|----------------------------------------|----------------------------------------------------------------------------|-----------------------------------------------------------------|
| [GLM](https://glm.g-truc.net)          | [MIT](https://glm.g-truc.net/copying.txt)                                  | branch: [`master`](https://github.com/g-truc/glm/tree/master)   |
| [stb](https://github.com/nothings/stb) | [MIT / Public Domain](https://github.com/nothings/stb/blob/master/LICENSE) | branch: [`master`](https://github.com/nothings/stb/tree/master) |

All libraries are used as `static library` to maximize optimization and limit problems with deployment and versions.

## Formats

- IdTech 2 (Quake, Quake 2)
  - MAP - in-development map (text)
    - Can be read by GoldSrc MAP reader
    - Exporting to this type is possible but not complete
- GoldSrc (Half-Life, Counter-Strike...)
  - BSP (v30) - compiled map
    - Extract/Export:
      - Textures
        - RGB/RGBA or raw data with palette
        - Generate WAD
      - Entity configuration
      - Map into .obj
        - Mtl with correct UV
        - ~~Separate collision map (clipping)~~
      - ~~Lightmap blob as texture~~
      - Nodes with map
        - Map components
        - ~~Visibility~~
          - Simple per-face textures
      - ~~Command entities as scripts~~
  - WAD (WAD2 + WAD3) - data (texture) storage
    - Textures
      - Raw data with palette
      - RGB and RGBA export supported
    - Images
    - Fonts
  - MAP - in-development map (text)
  - RMF - in-development map (binary)
  - FGD - entity definitions
    - See `Source` implementation which is extension of this format
- Source
  - FGD - entity definitions
      
## Compile & Use

Compilation tested using GCC and MinGW, C++20 support required.

Look into `tests` directory for different usages.

`GoldSrc` tests expect `half-life` directory (the one containing `valve`, `cstrike`, `platform`...) in project directory (same as `lib`, `src`...).
It is recommended to keep it where it is and use [Symbolic link](https://en.wikipedia.org/wiki/Symbolic_link).

## Command-line utility

Enabled by default, `set(DECAY_LIBRARY_CMD=OFF)` in your `CMakeLists.txt` to disable.

For list of commands, see [Commands.md](Commands.md).

## Linux tools

Inside `linux` directory, there are [MIME type](https://en.wikipedia.org/wiki/Media_type#Mime.types) definitions for supported BSP and WAD files for [KDE](https://kde.org/).

They recommend having only 1 `.xml` file per application but several are used sor simplicity.

| MIME                        | Definition File     | File type                       |
|-----------------------------|---------------------|---------------------------------|
| `application/goldsrc-bsp30` | `goldsrc-bsp30.xml` | BSP version 30                  |
| `application/goldsrc-fgd`   | `goldsrc-fgd.xml`   | FGD (entity definition)         |
| `application/goldsrc-map`   | `goldsrc-map.xml`   | In-development map (text)       |
| `application/goldsrc-rmf`   | `goldsrc-rmf.xml`   | In-development map (binary)     |
| `application/goldsrc-wad2`  | `goldsrc-wad2.xml`  | WAD 2 - textures, images, fonts |
| `application/goldsrc-wad3`  | `goldsrc-wad3.xml`  | WAD 3 - textures, images, fonts |

`*-bsp30` and `*-wad*` have weight `80` (same as `application/x-doom`) but are defined by file headers.

## Game Engines

Decay Library does not support and is not supported by any game engine.

In the future, it is planned to use [C# wrapper](https://github.com/AbitTheGray/Decay-Library_Csharp) for [Unity3D](https://unity.com/) (with additional scripts to create, for example, working doors).

Version for [Unreal Engine](https://www.unrealengine.com/en-US/) should be possible but is currently not planned.
If (or When) it will be in making, it will contain both C++ and Blueprint access.
