# Decay Lib

C++ library for GoldSrc (and other BSP-compatible) engines and formats.

All example C++ code is simplified for easier reading.

## Supported formats

Documentation below corresponds to supported features.

This helps with both development of this library and preservation of history.

- **GoldSrc**
  - [BSP](GoldSrc/BSP.md) `v30` / `0x1E`
    - Binary Space Partitioning - Map
    - [Valve Developer Community](https://developer.valvesoftware.com/wiki/BSP)
    - Map format
  - [WAD](GoldSrc/WAD.md) 2 + 3
    - Where's All Data
    - Content: Textures, Fonts, Images
  - [SPR](GoldSrc/SPR.md)
    - Sprite
  - [MDL](GoldSrc/MDL.md)
    - Skeletal / Animated Model
  - [FGD](Source/FGD.md) (**Source** version with marked differences for **GoldSrc**)
    - Forge Game Data
    - Definitions for entities in BSP (map)
  - [RMF](GoldSrc/RMF.md)
    - Rich Map Format
    - [Valve Developer Community](https://developer.valvesoftware.com/wiki/Rich_Map_Format)
    - Map development format from [Valve Hammer Editor](https://developer.valvesoftware.com/wiki/Valve_Hammer_Editor)
  - [MAP](GoldSrc/MAP.md)
    - Text format defining map geometry
    - [Valve Developer Community](https://developer.valvesoftware.com/wiki/MAP_file_format)
    - Map development format, predecessor for [RMF](GoldSrc/RMF.md)
  - [SC](GoldSrc/SC.md)
    - Client-side events
- **IdTech 2** (Quake 1/2) - https://www.gamers.org/dEngine/quake/spec/quake-spec34/
  - Bsp `v28` / `0x1C`
    - `Quake Shareware version, 22 June 96`
  - WAD 2
    - Same as [GoldSrc WAD 3](GoldSrc/WAD.md) but contains more types of entries
  - MAP - very similar to [GoldSrc](GoldSrc/MAP.md) (which is based on it)
- **Source**
  - BSP
    - Map
  - VMT
    - Material
  - VMF
  - VTF
    - Texture
  - MDL
    - Model
  - [FGD](Source/FGD.md)
    - Forge Game Data
    - Definitions for entities in BSP (map)

__If the link does not work (or there is none), it is not yet documented/supported.__
