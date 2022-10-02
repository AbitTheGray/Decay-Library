# Commands

## Help
`help`

Display information how to use the command-line utility.

Use other command after `help` (for example `help bsp2obj`) to show help for the specific command.

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

| Argument                                 | Required | Multiple | Description                                       |
|------------------------------------------|:--------:|:--------:|---------------------------------------------------|
| `--file <map.bsp>`                       |    ✓     |          | Source BSP map file                               |
| `--wad <map.wad>`                        |          |          | Add (or create) packed textures from BSP into WAD |
| `--newbsp <new_map.bsp>`                 |          |          | Save BSP map with no packed textures              |
| `--newbspwad <\half-life\valve\map.bsp>` |          |          | Path to add into map's "wad" path                 |

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

| Argument                      | Required | Multiple | Description                                       |
|-------------------------------|:--------:|:--------:|---------------------------------------------------|
| `--file <file.wad>`           |    ✓     |          | Source WAD file                                   |
| `--texture <texture.png>`     |          |    ✓     | Textures to add to the BSP                        |
| `--image <image.png>`         |          |    ✓     | ~~Image to add to the BSP~~                       |
| ~~`--font_atlas <font.png>`~~ |          |    ✓     | ~~16x16 font characters image to add to the BSP~~ |

- Modifies file from `--file` (the original file), there is no `--output` which would save a copy
- `--texture` is not needed after other arguments (=at the end) to allow you to add multiple textures easier
  - `--file map.bsp texture1.png texture2.png texture3.png`
  - `--file map.bsp --font_atlas font1.png texture1.png texture2.png texture3.png`

## Optimize WAD
~~`wad_optimize`~~ - NOT IMPLEMENTED

| Argument            | Required | Multiple | Description     |
|---------------------|:--------:|:--------:|-----------------|
| `--file <file.wad>` |    ✓     |          | Source WAD file |
| `--out <file.wad>`  |          |          | Output WAD file |

## MAP -> OBJ
~~`map_obj`~~ - NOT IMPLEMENTED

| Argument            | Required | Multiple | Description                                      |
|---------------------|:--------:|:--------:|--------------------------------------------------|
| `--file <file.map>` |    ✓     |          | Source MAP file                                  |
| `--obj <map.obj>`   |          |          | Path where to save Wavefront OBJ file            |
| `--mtl <map.mtl>`   |          |          | Path where to save Wavefront OBJ file's material |

### MAP -> RMF
`map2rmf`

| Argument            | Required | Multiple | Description     |
|---------------------|:--------:|:--------:|-----------------|
| `--file <file.map>` |    ✓     |          | Source MAP file |
| `--rmf <file.rmf>`  |    ✓     |          | Output RMF file |

- Use `rmf2map` for conversion in the opposite direction
- This conversion won't lose any data and will decrease file size (because of conversion from text file to binary file)

### RMF -> MAP
`rmf2map`

| Argument            | Required | Multiple | Description                                      |
|---------------------|:--------:|:--------:|--------------------------------------------------|
| `--file <file.rmf>` |    ✓     |          | Source RMF file                                  |
| `--map <file.map>`  |    ✓     |          | Output MAP file                                  |
| `--goldsrc`         |          |          | Force GoldSrc variant of MAP (currently default) |
| `--idtech2`         |          |          | Force IdTech2 variant of MAP                     |

- Use `map2rmf` for conversion in the opposite direction
- There is no advantage in this conversion - you loose part of the information (which is not needed for compilation), not gain anything and file size will increase (binary -> text)
- Using both `--goldsrc` and `--idtech2` is not valid

## FGD operations
`fgd`

| Argument                    | Required | Multiple | Description                                            |
|-----------------------------|:--------:|:--------:|--------------------------------------------------------|
| `--file <file.fgd>`         |          |          | Source FGD file                                        |
| `--add <file.fgd>`          |          |    ✓     | Add other FGD into current one (new priority)          |
| `--include <file.fgd>`      |          |    ✓     | Add other FGD into current one (old priority)          |
| `--subtract <file.fgd>`     |          |    ✓     | Subtract other FGD from current one                    |
| `--process_includes`        |          |          | Process `@Include`s                                    |
| `--include_dir <directory>` |          |          | Working directory for `@Include`s, implies `--include` |
| `--process_base`            |          |          | Process `base(...)` and discard `@BaseClass` classes   |
| `--output <file.fgd>`       |    ~     |          | Output as FGD file                                     |
| `--output_json <file.json>` |          |          | Output as JSON file                                    |

- Can process `@Include` to include referenced files
  - No need to use `--includes` when you use `--include_dir`
- Use `--json` only if you have problems reading `fgd` file and want to lear as no command accepts JSON FGD

## WAD Find
~~`wad_find`~~ - NOT IMPLEMENTED

| Argument                   | Required | Multiple | Description                        |
|----------------------------|:--------:|:--------:|------------------------------------|
| `--dir <game_directory>`   |          |          | Directory to search                |
| `--item <item_name>`       |          |          | Name of WAD item to search for     |
| `--texture <texture_name>` |          |          | Same as `--item` but only textures |
| `--image <image_name>`     |          |          | Same as `--item` but only image    |
| `--font <font_name>`       |          |          | Same as `--item` but only font     |

- If you do not specify `--dir`, current working directory will be used
- You can search only for 1 item at a time
  - = cannot combine `--item`, `--texture`, `--image` or `--font`
