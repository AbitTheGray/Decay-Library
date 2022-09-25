# Commands

## Help
`help`

Display information how to use the command-line utility.

## BSP -> OBJ
`bsp2obj`

| Argument                         | Required | Description                           |
|----------------------------------|:--------:|---------------------------------------|
| `--file <map.bsp>`               |    ✓     | Source BSP map file                   |
| `--obj <map.obj>`                |          | 3D model of the map                   |
| `--mtl <map.mtl>`                |          | Material + texture assignment file    |
| `--textures <texture_directory>` |          | Directory to store extracted textures |

## BSP -> WAD
`bsp2wad`

| Argument                                     | Required | Description                                       |
|----------------------------------------------|:--------:|---------------------------------------------------|
| `--file <map.bsp>`                           |    ✓     | Source BSP map file                               |
| `--wad <map.wad>`                            |          | Add (or create) packed textures from BSP into WAD |
| `--newbsp <new_map.bsp>`                     |          | Save BSP map with no packed textures              |
| ~~`--newbspwad <\half-life\valve\map.bsp>`~~ |          | ~~Path to add into map's "wad" path~~             |

## BSP Lightmap
`bsp_lightmap`

| Argument                    | Required | Description                      |
|-----------------------------|:--------:|----------------------------------|
| `--file <map.bsp>`          |    ✓     | Source BSP map file              |
| `--lightmap <lightmap.png>` |          | Where to save generated lightmap |

## BSP Entity
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

## Add texture to WAD
`wad_add`

| Argument                  | Required | Description                                       |
|---------------------------|:--------:|---------------------------------------------------|
| `--file <file.wad>`       |    ✓     | Source WAD file                                   |
| `--texture <texture.png>` |          | Textures to add to the BSP                        |
| ~~`--font <font.png>`~~   |          | ~~16x16 font characters image to add to the BSP~~ |
| ~~`--image <image.png>`~~ |          | ~~Image to add to the BSP~~                       |

- `--textures` is not needed after other arguments to allow you to add multiple textures easier
    - example: `--file map.bsp texture1.png texture2.png texture3.png`

## Optimize WAD
~~`wad_optimize`~~ - NOT IMPLEMENTED

| Argument            | Required | Description                                       |
|---------------------|:--------:|---------------------------------------------------|
| `--file <file.wad>` |    ✓     | Source WAD file                                   |
| `--out [file.wad]`  |          | Textures to add to the BSP                        |

## MAP -> OBJ
~~`map_obj`~~ - NOT IMPLEMENTED

### MAP -> RMF
~~`map2rmf`~~ - NOT IMPLEMENTED

- use `rmf2map` for conversion in the opposite direction
- This conversion won't loose any data and will decrease file size (because of conversion from text file to binary file)

### RMF -> MAP
~~`rmf2map`~~ - NOT IMPLEMENTED

- use `map2rmf` for conversion in the opposite direction
- There is no advantage in this conversion - you loose part of the information (which is not needed for compilation), not gain anything and file size will increase 

## FGD -> JSON
~~`fgd_json`~~ - NOT IMPLEMENTED

## Combine FGD files
~~`fgd_combine`~~ - NOT IMPLEMENTED

- Can also process `@Include` to include referenced files
