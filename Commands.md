# Commands

## Help
`help`

Display information how to use the command-line utility.

## BSP -> OBJ
`bsp2obj`

| Argument                         | Required | Multiple | Description                           |
|----------------------------------|:--------:|:--------:|---------------------------------------|
| `--file <map.bsp>`               |    ✓     |          | Source BSP map file                   |
| `--obj <map.obj>`                |          |          | 3D model of the map                   |
| `--mtl <map.mtl>`                |          |          | Material + texture assignment file    |
| `--textures <texture_directory>` |          |          | Directory to store extracted textures |

## BSP -> WAD
`bsp2wad`

| Argument                                     | Required | Multiple | Description                                       |
|----------------------------------------------|:--------:|:--------:|---------------------------------------------------|
| `--file <map.bsp>`                           |    ✓     |          | Source BSP map file                               |
| `--wad <map.wad>`                            |          |          | Add (or create) packed textures from BSP into WAD |
| `--newbsp <new_map.bsp>`                     |          |          | Save BSP map with no packed textures              |
| ~~`--newbspwad <\half-life\valve\map.bsp>`~~ |          |          | ~~Path to add into map's "wad" path~~             |

## BSP Lightmap
`bsp_lightmap`

| Argument                    | Required | Multiple | Description                      |
|-----------------------------|:--------:|:--------:|----------------------------------|
| `--file <map.bsp>`          |    ✓     |          | Source BSP map file              |
| `--lightmap <lightmap.png>` |          |          | Where to save generated lightmap |

## BSP Entity
`bsp_entity`

| Argument                         | Required | Multiple | Description                                                                                    |
|----------------------------------|:--------:|:--------:|------------------------------------------------------------------------------------------------|
| `--file <map.bsp>`               |          |          | Source BSP map file                                                                            |
| `--replace <entities>`           |          |          | Replace entity info in the map                                                                 |
| `--replace_json <entities.json>` |          |          | Replace entity info in the map using JSON-formatted entity info                                |
| `--add <entities>`               |          |    ✓     | Add entity info at the end of existing entity info in the map                                  |
| `--add_json <entities.json>`     |          |    ✓     | Add entity info (JSON) at the end of existing entity info in the map                           |
| `--validate <gamemode.fgd>`      |          |          | Validate map's entities against FGD and print additional/missing values (into standard output) |
| `--extract <entities>`           |          |          | Extract entity info from the map                                                               |
| `--extract_json <entities.json>` |          |          | Extract entity info from the map as JSON                                                       |
| ~~`--outbsp <map.bsp>`~~         |          |          | ~~Save changed entities into, requires `--file`~~                                              |

- Make sure you reference same entities (use correct model IDs) when using `--add` and `--replace` (or their JSON variants)
- Arguments are processed in order `--file` -> `--replace` -> `--add` -> `--validate` -> `--extract`
- Either `--outbsp` or `--extract` is required to save the data
- JSON variants require [nlohmann's JSON](https://github.com/nlohmann/json) library (see option `DECAY_JSON_LIB` inside [CMakeLists.txt](CMakeLists.txt))
- Using `--replace` and `--replace_json` at the same time will cause "undefined behaviour"

## Add texture to WAD
`wad_add`

| Argument                  | Required | Multiple | Description                                       |
|---------------------------|:--------:|:--------:|---------------------------------------------------|
| `--file <file.wad>`       |    ✓     |          | Source WAD file                                   |
| `--texture <texture.png>` |          |    ✓     | Textures to add to the BSP                        |
| ~~`--font <font.png>`~~   |          |    ✓     | ~~16x16 font characters image to add to the BSP~~ |
| ~~`--image <image.png>`~~ |          |    ✓     | ~~Image to add to the BSP~~                       |

- `--textures` is not needed after other arguments to allow you to add multiple textures easier
    - example: `--file map.bsp texture1.png texture2.png texture3.png`

## Optimize WAD
~~`wad_optimize`~~ - NOT IMPLEMENTED

| Argument            | Required | Multiple | Description                |
|---------------------|:--------:|:--------:|----------------------------|
| `--file <file.wad>` |    ✓     |          | Source WAD file            |
| `--out [file.wad]`  |          |          | Textures to add to the BSP |

## MAP -> OBJ
~~`map_obj`~~ - NOT IMPLEMENTED

### MAP -> RMF
`map2rmf`

| Argument            | Required | Multiple | Description     |
|---------------------|:--------:|:--------:|-----------------|
| `--file <file.map>` |    ✓     |          | Source MAP file |
| `--rmf [file.rmf]`  |    ✓     |          | Output RMF file |

- use `rmf2map` for conversion in the opposite direction
- This conversion won't lose any data and will decrease file size (because of conversion from text file to binary file)

### RMF -> MAP
`rmf2map`

| Argument            | Required | Multiple | Description                                          |
|---------------------|:--------:|:--------:|------------------------------------------------------|
| `--file <file.rmf>` |    ✓     |          | Source RMF file                                      |
| `--map [file.map]`  |    ✓     |          | Output MAP file                                      |
| ~~`--goldsrc`~~     |          |          | ~~Force GoldSrc variant of MAP (currently default)~~ |
| ~~`--idtech2`~~     |          |          | ~~Force IdTech2 variant of MAP~~                     |

- use `map2rmf` for conversion in the opposite direction
- There is no advantage in this conversion - you loose part of the information (which is not needed for compilation), not gain anything and file size will increase (binary -> text)

## FGD -> JSON
~~`fgd_json`~~ - NOT IMPLEMENTED

## Combine FGD files
~~`fgd_combine`~~ - NOT IMPLEMENTED

- Can also process `@Include` to include referenced files
