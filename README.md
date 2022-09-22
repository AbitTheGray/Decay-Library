# Decay Library

Library for parsing BSP and WAD files from GoldSource (and later Quake and Source) engine.

Also contains C API which can be used from other programming languages.
[C# wrapper](https://github.com/AbitTheGray/Decay-Library_Csharp) is being worked on (for future use in Unity3D) but may lag-behind in features.

Can also be used as Command-line program.

## Goal

Goal of this project is to learn, test and document.

Learn = get to know how others approached the problem before and what decisions they made.

Test = write a code that can interpret the data, see whenever it can be done easier today, validate files.

Document = write documentation for others and "share the knowledge" (as did people before this project).

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

## Command-line utility

Enabled by default, `set(DECAY_LIBRARY_CMD=OFF)` in your `CMakeLists.txt` to disable.

### Help
`help`

Display information how to use the command-line utility.

### BSP -> OBJ
`bsp2obj`

| Argument                         | Required | Description                           |
|----------------------------------|:--------:|---------------------------------------|
| `--file <map.bsp>`               |    ✓     | Source BSP map file                   |
| `--obj <map.obj>`                |          | 3D model of the map                   |
| `--mtl <map.mtl>`                |          | Material + texture assignment file    |
| `--textures <texture_directory>` |          | Directory to store extracted textures |

### BSP -> WAD
`bsp2wad`

| Argument                                     | Required | Description                                       |
|----------------------------------------------|:--------:|---------------------------------------------------|
| `--file <map.bsp>`                           |    ✓     | Source BSP map file                               |
| `--wad <map.wad>`                            |          | Add (or create) packed textures from BSP into WAD |
| `--newbsp <new_map.bsp>`                     |          | Save BSP map with no packed textures              |
| ~~`--newbspwad <\half-life\valve\map.bsp>`~~ |          | ~~Path to add into map's "wad" path~~             |

### BSP Lightmap
`bsp_lightmap`

| Argument                    | Required | Description                      |
|-----------------------------|:--------:|----------------------------------|
| `--file <map.bsp>`          |    ✓     | Source BSP map file              |
| `--lightmap <lightmap.png>` |          | Where to save generated lightmap |

### BSP Entity
~~`bsp_entity`~~ - NOT IMPLEMENTED

| Argument                         | Required | Description                                                                                    |
|----------------------------------|:--------:|------------------------------------------------------------------------------------------------|
| `--file <map.bsp>`               |    ✓     | Source BSP map file                                                                            |
| `--extract <entities>`           |          | Extract entity info from the map                                                               |
| `--extract_json <entities.json>` |          | Extract entity info from the map as JSON                                                       |
| `--add <entities>`               |          | Add entity info at the end of existing entity info in the map                                  |
| `--add_json <entities.json>`     |          | Add entity info (JSON) at the end of existing entity info in the map                           |
| `--replace <entities>`           |          | Replace entity info in the map                                                                 |
| `--replace_json <entities.json>` |          | Replace entity info in the map using JSON-formatted entity info                                |
| `--validate <gamemode.fgd>`      |          | Validate map's entities against FGD and print additional/missing values (into standard output) |

- Make sure you reference same entities when using `--add` and `--replace` (or their JSON variants)

### Add texture to WAD
`wad_add`

| Argument                  | Required | Description                                       |
|---------------------------|:--------:|---------------------------------------------------|
| `--file <file.wad>`       |    ✓     | Source WAD file                                   |
| `--texture <texture.png>` |          | Textures to add to the BSP                        |
| ~~`--font <font.png>`~~   |          | ~~16x16 font characters image to add to the BSP~~ |
| ~~`--image <image.png>`~~ |          | ~~Image to add to the BSP~~                       |

- `--textures` is not needed after other arguments to allow you to add multiple textures easier
    - example: `--file map.bsp texture1.png texture2.png texture3.png`

### Optimize WAD
~~`wad_optimize`~~ - NOT IMPLEMENTED

| Argument            | Required | Description                                       |
|---------------------|:--------:|---------------------------------------------------|
| `--file <file.wad>` |    ✓     | Source WAD file                                   |
| `--out [file.wad]`  |          | Textures to add to the BSP                        |

### MAP -> OBJ
~~`map_obj`~~ - NOT IMPLEMENTED

### FGD -> JSON
~~`fgd_json`~~ - NOT IMPLEMENTED

### Combine FGD files
~~`fgd_combine`~~ - NOT IMPLEMENTED

- Can also process `@Include` to include referenced files

## Linux tools

Inside `linux` directory, there are [MIME type](https://en.wikipedia.org/wiki/Media_type#Mime.types) definitions for supported BSP and WAD files for [KDE](https://kde.org/).

They recommend having only 1 `.xml` file per application but several are used sor simplicity.

| MIME                        | Definition File     | File type      |
|-----------------------------|---------------------|----------------|
| `application/goldsrc-bsp30` | `goldsrc-bsp30.xml` | BSP version 30 |
| `application/goldsrc-wad2`  | `goldsrc-wad2.xml`  | WAD2           |
| `application/goldsrc-wad3`  | `goldsrc-wad3.xml`  | WAD3           |
| `application/goldsrc-map`   | `goldsrc-map.xml`   | MAP            |
| `application/goldsrc-fgd`   | `goldsrc-fgd.xml`   | MAP            |

`bsp30` and `wad*` have weight `80` (same as `application/x-doom`) but are defined by file headers.

## Game Engines

Decay Library does not support and is not supported by any game engine.

In the future, it is planned to use [C# wrapper](https://github.com/AbitTheGray/Decay-Library_Csharp) for [Unity3D](https://unity.com/) (with additional scripts to create, for example, working doors).

Version for [Unreal Engine](https://www.unrealengine.com/en-US/) should be possible but is currently not planned.
If (or When) it will be in making, it will contain both C++ and Blueprint access.
