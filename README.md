# Decay Library

Library for parsing BSP and WAD files from GoldSource (and maybe more later).

~~Can also used as Command-line program.~~

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
      - ~~Textures~~
        - ~~RGB/RGBA or raw data with palette~~
        - ~~Generate WAD~~
      - ~~Entity configuration~~
      - ~~Map into .obj~~
        - ~~Mtl with correct UV~~
        - ~~Separate collision map~~
      - ~~Lightmap blob as texture~~
      - ~~Nodes with map~~
        - ~~Map components~~
        - ~~Visibility~~
      - ~~Command entities as scripts~~
  - WAD (WAD2 + WAD3)
    - Textures
      - Raw data with palette
      - RGB and RGBA export supported
    - Images
      - Some seem weird but it may be correct
    - ~~Fonts~~
      - **Currently fail to load, requires fixing**
