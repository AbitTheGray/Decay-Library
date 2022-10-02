# TODO List

- General
  - `R_ASSERT` / `D_ASSERT`
    -[ ] Look for places where `R_ASSERT` can simplify the code (instead of `if` + `throw`)
  -[ ] Verify all formats in Valve Hammer Editor
- FGD
  -[ ] Test for `Fgd::FgdFile::Subtract(...)`
  -[ ] `Fgd::FgdFile::Add(...)`, `Fgd::FgdFile::Subtract(...)` and `Fgd::FgdFile::Include(...)` should look at base classes
    - Verify whenever it is needed
    - Maybe also `Fgd::FgdFile::ProcessIncludes(...)`
- MAP
  -[ ] Conversion of `Map::MapFile`'s brush into polygon-based (or triangulated) object (including texture coordinates).
  -[ ] Utility functions to simplify entity manipulation / polygon conversion
  -[ ] Document in [MAP.md](docs/GoldSrc/MAP.md) how to convert brush into polygons
  -[ ] Cleanup entity properties using [FGD file](docs/Source/FGD.md)
- Commands
  -[ ] Add [`fgd` command](Commands.md#fgd-operations)
  -[ ] Move command `bsp_lightmap` into `bsp2obj`
