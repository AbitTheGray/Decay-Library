# MAP

`*.map` file is text file defining map structure.
This format was used by `IdTech 2` and `GoldSrc` engines but the latter usually used [`*.rmf`](RMF.md).

Map itself consists of 1 or more entities.

The `classname`=`worldspawn` must always be present (can be twice inside same entity) and **should** be the first entity in the file.
This `worldspawn` entity contains brushes of all "non-entity" objects in the map.

## Entity

Each entity starts by `{` and end by corresponding `}`, no other character outside entity is valid.

Inside each entity are parameters and brushes.

### Parameter

Parameter is in format `"key" "value"` (one per line) where `key` should be from [FGD file](../Source/FGD.md) (but custom ones are also possible but no guarantee that the game will understand it).

Every entity must contain `"classname"` key to define type of the entity (otherwise the map is invalid).
The same entity will also contain `"wad"` like `"\half-life\cstrike\cstrike.wad;\half-life\cstrike\cs_dust.wad"` - path where `\half-life\` is game directory, values are separated by `;` and `\` is not an escape character.

### Brush

Brush starts by `{` and end by corresponding `}` and contain 1 plane per line with minimum of 4 planes (otherwise cannot construct 3D object).

#### Plane

A line of Brush definition can look like this, and it describes a plane in 3D space.
```
( -64 64 0 ) ( 64 64 0 ) ( 64 -64 0 ) ASPHALT01 [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
```

- `( -64 64 0 ) ( 64 64 0 ) ( 64 -64 0 )` - 3 vertex coordinates
  - Only whole numbers
  - Must not form a line (cross of directional vectors from one vertex to the other two vertices)
- `ASPHALT01` - texture of the face
- `[ 1 0 0 0 ] [ 0 -1 0 0 ]` - U + V texture offset
  - For `IdTech 2` this would be just `0 0`
- `0` - Texture rotation
  - Not used for calculation, only for editor to know current rotation
- `1 1` - Texture scale
  - Only this (officially) supports decimal values
