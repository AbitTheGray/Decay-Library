# TODO List

- General
  -[ ] Add messages into `R_ASSERT` and `D_ASSERT`
  -[ ] Look for places where `R_ASSERT` can simplify the code (instead of `if` + `throw`)
  -[ ] Replace `R_ASSERT` by `D_ASSERT` where it is only for comfort and not really needed
- FGD
  -[ ] Test for `Fgd::FgdFile::ProcessIncludes(...)`
  -[ ] Test for `Fgd::FgdFile::Subtract(...)`
- MAP
  -[ ] Conversion of `Map::MapFile`'s brush into polygon-based (or triangulated) object (including texture coordinates).
  -[ ] Utility functions to simplify entity manipulation / polygon conversion
  -[ ] Document in [MAP.md](docs/GoldSrc/MAP.md) how to convert brush into polygons
  -[ ] Cleanup entity properties using [FGD file](docs/Source/FGD.md)
- Commands
  -[ ] WIP - Rework implementation
  -[ ] Add FGD commands
  -[ ] Add MAP commands
